#ifndef HOOKFS_SIMPLEIOSTREAM_H
#define HOOKFS_SIMPLEIOSTREAM_H

#include <stddef.h>

#include <simplestring.h>

/**
 * @addtogroup Hookfs
 * @{
 */


struct SimpleIOStream {
  struct SimpleString;

  int i;
  FILE *f;
}


int sioprintf (
    struct SimpleString * restrict s, const char * restrict format, ...)
  __attribute__((format(printf, 2, 3)));
size_t siowrite (
    const void * restrict ptr, size_t size, size_t count,
    struct SimpleString * restrict s);
int sioscanf (
    struct SimpleString * restrict s, const char * restrict format, ...)
  __attribute__((format(scanf, 2, 3)));
size_t sioread (
    const void * restrict ptr, size_t size, size_t count,
    struct SimpleString * restrict s);

/**@}*/


#endif /* HOOKFS_SIMPLEIOSTREAM_H */
