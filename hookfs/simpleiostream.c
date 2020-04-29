#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <macro.h>

#include "simpleiostream.h"


size_t siowrite (
    const void * restrict ptr, size_t size, size_t count,
    GString * restrict s) {
  size *= count;
  g_string_append_len(s, ptr, size);
  return size;
}


int sioscanf (
    struct SimpleIStream * restrict s, const char * restrict format, ...) {
  fseek(s->f, s->i, SEEK_SET);
  va_list args;
  va_start(args, format);
  int ret = fscanf(s->f, format, args);
  va_end(args);
  s->i = ftell(s->f);
  return ret;
}


size_t sioread (
    void * restrict ptr, size_t size, size_t count,
    struct SimpleIStream * restrict s) {
  size *= count;
  if (s->i + size > s->len) {
    size = s->len - s->i;
  }
  memcpy(ptr, s->data + s->i, size);
  s->i += size;
  return size;
}


void SimpleIStream_destroy (struct SimpleIStream *s) {
  fclose(s->f);
  free(s->data);
}


int SimpleIStream_init (struct SimpleIStream *s, void *ptr, size_t size) {
  s->data = ptr;
  s->len = size;
  s->i = 0;
  s->f = fmemopen(ptr, size, "r");
  return 0;
}
