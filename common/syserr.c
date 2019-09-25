#include "malloc.h"


extern inline int SystemCallException_init (
    SystemCallException *e, const char *file, const char *func, unsigned line,
    char *what);


static int Exception_fputs_systemcall (const BaseException *e_, FILE *stream) {
  const Exception *e = (const Exception *) e_;

  int ret = fputs("Fail to allocate memory", stream);
  if (e->what) {
    ret += fputs(" for ", stream);
    ret += fputs(e->what, stream);
  }
  ret += fputs(", possible out of memory\n", stream);
  return ret;
}


VTABLE_INIT(Exception, SystemCallException) = {
  .name = "SystemCallException",
  .destory = NULL,
  .fputs = Exception_fputs_systemcall
};


extern inline void *malloc_e_snx (size_t size, const char *file, const char *func, unsigned line, char *name);
