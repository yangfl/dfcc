#ifndef SYSERR_H
#define SYSERR_H

#include <errno.h>
#include <stdlib.h>

#include "try.h"


ExceptionDef(SystemCallException, {
  char *what;
  int errno_;
});


inline int SystemCallException_init (
    SystemCallException *e, const char *file, const char *func, unsigned line,
    char *what) {
  int res = Exception_init((Exception *) e, file, func, line, 0);
  if unlikely (res) {
    return res;
  }

  e->what = what;
  e->errno_ = errno;
  e->VTABLE(Exception) = &VTABLE_OF(Exception, SystemCallException);
  return 0;
}

#define SystemCallException(msg) GENERATE_EXCEPTION_INIT(SystemCallException, msg)

inline void *malloc_e_snx (size_t size, const char *file, const char *func, unsigned line, char *name) {
  void *ret = malloc(size);
  if unlikely (ret == NULL) {
    SystemCallException_init(&ex, file, func, line, name);
  }
  return ret;
}

#define malloc_e_sn(size, name) malloc_e_snx(size, __FILE__, __func__, __LINE__, name)
#define malloc_e_s(size) malloc_e_snx(size, __FILE__, __func__, __LINE__, NULL)
#define malloc_e_generic(...) GET_3RD_ARG(__VA_ARGS__, malloc_e_sn, malloc_e_s, )
#define malloc_e(...) malloc_e_generic(__VA_ARGS__)(__VA_ARGS__)

#define malloc_et(type) ((type *) malloc_e_sn(sizeof(type), # type))
#define malloc_t(type) ((type *) malloc(sizeof(type)))

#define WRAP_SYSCALL()


#endif /* SYSERR_H */
