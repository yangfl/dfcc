#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <threads.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "_class.h"


#define BT_BUF_SIZE 32

typedef struct BaseException BaseException;

#define EXCEPTION_MASKED 2

typedef int (*Exception_fputs_t) (const BaseException *, FILE *);
typedef void (*Exception_destory_t) (BaseException *);


CLASS(BaseException, {
  char set;
  int nptrs;
  unsigned line;
  const char *file;
  const char *func;
  void *bt[BT_BUF_SIZE];
  VTABLE(Exception, {
    const char *name;
    Exception_fputs_t fputs;
    Exception_destory_t destory;
  });
});

CLASS(Exception, {
  INHERIT(BaseException);
  char *what;
  char __padding[1024 - sizeof(struct BaseException) - sizeof(char *)];
});

#define ExceptionDef(name, ...) \
  typedef struct name { \
    struct BaseException; \
    struct __VA_ARGS__; \
  } name; \
  _Static_assert(sizeof(name) <= sizeof(Exception), # name " too large"); \
  VTABLE_IMPL(Exception, name)

extern thread_local Exception ex;

#define GENERATE_EXCEPTION_INIT(type, ...) type ## _init((type *) &ex, __FILE__, __func__, __LINE__, ## __VA_ARGS__)

void Exception_copy (Exception *dst, const Exception *src);
int Exception_fputs (const Exception *e, FILE *stream);
int Exception_fputs_what (const BaseException *e_, FILE *stream);

inline bool Exception_has (const Exception *e) {
  return e->set == true;
}

void Exception_dirtydestory (Exception *e);
void Exception_destory (Exception *e);
void Exception_destory_dynamic (BaseException *e_);
int Exception_init (Exception *e, const char *file, const char *func, unsigned line, bool no_backtrace);

#define TEST_SUCCESS Exception_has(&ex)


/* unspecified */

typedef Exception UnspecifiedException;
VTABLE_IMPL(Exception, UnspecifiedException);

inline int UnspecifiedException_init (
    UnspecifiedException *e, const char *file, const char *func, unsigned line,
    const char *what) {
  int res = Exception_init((Exception *) e, file, func, line, 0);
  if unlikely (res) {
    return res;
  }

  e->what = strdup(what);
  e->VTABLE(Exception) = &VTABLE_OF(Exception, UnspecifiedException);
  return 0;
}

#define UnspecifiedException(msg) GENERATE_EXCEPTION_INIT(UnspecifiedException, msg)
#define Quick UnspecifiedException("Quick exception")


#endif /* EXCEPTION_H */
