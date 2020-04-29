#ifndef TINYGLIB_H
#define TINYGLIB_H

#include <stdbool.h>
#include <stddef.h>


void *g_malloc (size_t n_bytes) __attribute__((malloc, __alloc_size__(1)));
void *g_realloc (void *mem, size_t n_bytes) __attribute__((warn_unused_result));
#define g_free free


typedef enum {
  /* log flags */
  G_LOG_FLAG_RECURSION          = 1 << 0,
  G_LOG_FLAG_FATAL              = 1 << 1,

  /* GLib log levels */
  G_LOG_LEVEL_ERROR             = 1 << 2,       /* always fatal */
  G_LOG_LEVEL_CRITICAL          = 1 << 3,
  G_LOG_LEVEL_WARNING           = 1 << 4,
  G_LOG_LEVEL_MESSAGE           = 1 << 5,
  G_LOG_LEVEL_INFO              = 1 << 6,
  G_LOG_LEVEL_DEBUG             = 1 << 7,

  G_LOG_LEVEL_MASK              = ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
} GLogLevelFlags;
extern GLogLevelFlags cur_level;


void g_log (const char *log_domain, GLogLevelFlags log_level,
            const char *format, ...) __attribute__((format(printf, 3, 4)));


typedef struct GArray {
  char *data;
  unsigned int len;
  unsigned int alloc;
  unsigned int elt_size;
  unsigned int zero_terminated : 1;
} GArray;


GArray *g_array_set_size (GArray *array, size_t length);
#define g_array_append_val(a,v) g_array_append_vals (a, &(v), 1)
GArray *g_array_append_vals (GArray *array, const void *data, size_t len);
#define g_array_index(a,t,i) (((t*) (void *) (a)->data) [(i)])
char *g_array_destroy (GArray *array, bool free_segment);
char *g_array_free (GArray *array, bool free_segment);
int g_array_init (
  GArray *array, bool zero_terminated, bool clear_, size_t element_size);
GArray *g_array_new (bool zero_terminated, bool clear_, size_t element_size);


typedef GArray GString;


GString *g_string_append_len (GString *string, const char *val, size_t len);
GString *g_string_append (GString *string, const char *val);
int g_string_append_vprintf (GString *string, const char *format, va_list args)
  __attribute__((format(printf, 2, 0)));
int g_string_append_printf (GString *string, const char *format, ...)
  __attribute__((format(printf, 2, 3)));
char *g_string_destroy (GString *string, bool free_segment);
char *g_string_free (GString *string, bool free_segment);
int g_string_init (GString *string, const char *init);
GString *g_string_new (const char *init);


#endif /* TINYGLIB_H */
