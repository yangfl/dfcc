#define _GNU_SOURCE
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macro.h"

#include "tinyglib.h"



static void __attribute__((noreturn)) fail_alloc (size_t n_bytes) {
  g_log("lib", G_LOG_LEVEL_CRITICAL,
        "Failed to allocate %zu bytes", n_bytes);
  exit(1);
}


void *g_malloc (size_t n_bytes) {
  if unlikely (n_bytes == 0) {
    return NULL;
  }

  void *mem = malloc(n_bytes);
  should (mem != NULL) otherwise {
    fail_alloc(n_bytes);
  }
  return mem;
}


void *g_realloc (void *mem, size_t n_bytes) {
  if unlikely (n_bytes == 0) {
    g_free(mem);
    return NULL;
  }

  mem = realloc(mem, n_bytes);
  should (mem != NULL) otherwise {
    fail_alloc(n_bytes);
  }
  return mem;
}


/********** log **********/


GLogLevelFlags cur_level = G_LOG_LEVEL_MESSAGE;


void g_log (const char *log_domain, GLogLevelFlags log_level,
            const char *format, ...) {
  if (log_level <= cur_level) {
    va_list ap;
    va_start(ap, format);
    if (log_level == G_LOG_LEVEL_ERROR || log_level == G_LOG_LEVEL_CRITICAL) {
      vfprintf(stderr, format, ap);
      fputc('\n', stderr);
    } else {
      vprintf(format, ap);
      putchar('\n');
    }
    va_end(ap);
  }
}


/********** array **********/


#define MIN_ARRAY_SIZE 16
#define g_array_elt_len(array,i) ((array)->elt_size * (i))
#define g_array_elt_pos(array,i) ((array)->data + g_array_elt_len((array),(i)))
#define g_array_elt_zero(array, pos, len) \
  (memset(g_array_elt_pos((array), pos), 0, g_array_elt_len((array), len)))
#define g_array_zero_terminate(array) { \
  if ((array)->zero_terminated) { \
    g_array_elt_zero((array), (array)->len, 1); \
  } \
}


static void g_array_maybe_expand (GArray *array, size_t len) {
  size_t want_alloc =
    g_array_elt_len(array, array->len + len + array->zero_terminated);

  if (want_alloc > array->alloc) {
    want_alloc = want_alloc / 2 * 3;
    if (MIN_ARRAY_SIZE > want_alloc) {
      want_alloc = MIN_ARRAY_SIZE;
    }

    array->data = g_realloc(array->data, want_alloc);

    array->alloc = want_alloc;
  }
}


static void g_array_maybe_shrink (GArray *array) {
  if (array->len < array->alloc / 4 * 3) {
    array->data = g_realloc(array->data, array->alloc / 2);
  }
}


GArray *g_array_set_size (GArray *array, size_t length) {
  if unlikely (length == array->len) {
    return array;
  }

  bool expand_array = length > array->len;
  if (expand_array) {
    g_array_maybe_expand(array, length - array->len);
  }
  array->len = length;
  if (!expand_array) {
    g_array_maybe_shrink(array);
  }

  g_array_zero_terminate(array);
  return array;
}


GArray *g_array_append_vals (GArray *array, const void *data, size_t len) {
  size_t old_len = array->len;
  g_array_set_size(array, array->len + len);
  memcpy(g_array_elt_pos(array, old_len), data, len * array->elt_size);
  return array;
}


char *g_array_destroy (GArray *array, bool free_segment) {
  char *segment;

  if (free_segment) {
    g_free(array->data);
    segment = NULL;
  } else {
    segment = array->data;
  }

  return segment;
}


char *g_array_free (GArray *array, bool free_segment) {
  char *segment = g_array_destroy(array, free_segment);
  g_free(array);
  return segment;
}


int g_array_init (
    GArray *array, bool zero_terminated, bool clear_, size_t element_size) {
  array->data            = NULL;
  array->len             = 0;
  array->alloc           = 0;
  array->zero_terminated = zero_terminated;
  array->elt_size        = element_size;

  if (array->zero_terminated) {
    g_array_maybe_expand(array, 0);
    g_array_zero_terminate(array);
  }

  return 0;
}


GArray *g_array_new (bool zero_terminated, bool clear_, size_t element_size) {
  GArray *array = g_malloc(sizeof(GArray));
  g_array_init(array, zero_terminated, clear_, element_size);
  return array;
}


/********** string **********/


GString *g_string_append_len (GString *string, const char *val, size_t len) {
  return (GString *) g_array_append_vals((GArray *) string, val, len);
}


GString *g_string_append (GString *string, const char *val) {
  return g_string_append_len(string, val, strlen(val));
}


int g_string_append_vprintf (GString *string, const char *format, va_list args) {
  char *buf;
  int ret = vasprintf(&buf, format, args);
  should (ret >= 0) otherwise {
    return ret;
  }

  g_string_append_len(string, buf, ret);
  return ret;
}


int g_string_append_printf (GString *string, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int ret = g_string_append_vprintf(string, format, args);
  va_end(args);
  return ret;
}


char *g_string_destroy (GString *string, bool free_segment) {
  return g_array_destroy((GArray *) string, free_segment);
}


char *g_string_free (GString *string, bool free_segment) {
  return g_array_free((GArray *) string, free_segment);
}


int g_string_init (GString *string, const char *init) {
  g_array_init((GArray *) string, true, false, sizeof(char));
  if (init != NULL) {
    g_string_append(string, init);
  }
  return 0;
}


GString *g_string_new (const char *init) {
  GString *string = g_malloc(sizeof(GString));
  g_string_init(string, init);
  return string;
}
