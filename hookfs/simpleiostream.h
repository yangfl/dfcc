#ifndef HOOKFS_SIMPLEIOSTREAM_H
#define HOOKFS_SIMPLEIOSTREAM_H

#include <stddef.h>

#include "common/tinyglib.h"


//! @ingroup Hookfs
struct SimpleIStream {
  char *data;
  FILE *f;
  unsigned int len;
  unsigned int i;
};


#define sioprintf g_string_append_printf
size_t siowrite (
  const void * restrict ptr, size_t size, size_t count,
  GString * restrict s);
int sioscanf (
  struct SimpleIStream * restrict s, const char * restrict format, ...)
  __attribute__((format(scanf, 2, 3)));
size_t sioread (
  void * restrict ptr, size_t size, size_t count,
  struct SimpleIStream * restrict s);
void SimpleIStream_destroy (struct SimpleIStream *s);
int SimpleIStream_init (struct SimpleIStream *s, void *ptr, size_t size);


#endif /* HOOKFS_SIMPLEIOSTREAM_H */
