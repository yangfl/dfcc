#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GLIBC__
#include <execinfo.h>
#endif

#include "class.h"

#include "exception.h"


thread_local Exception ex = {
  .set = false, .file = NULL, .line = 0, .VTABLE(Exception) = NULL, .what = NULL
};


static void __attribute__((destructor)) Exception_destructor (void) {
  if unlikely (ex.set) {
    fprintf(stderr, "Unhandled ");
    Exception_fputs(&ex, stderr);
    Exception_destory(&ex);
  }
}


void Exception_copy (Exception *dst, const Exception *src) {
  PROTECT_RETURN(dst);
  PROTECT_RETURN(src);

  if (dst != src) {
    if (src->set) {
      memcpy(dst, src, sizeof(Exception));
    } else {
      dst->set = false;
    }
  }
}


static int Exception_fputs_backtrace_symbols (void * const *bt, int size, FILE *stream) {
#ifdef __GLIBC__
  void *cur_bt[BT_BUF_SIZE + 2];
  int ret = 0;

  if (bt == NULL) {
    size = backtrace(cur_bt, BT_BUF_SIZE + 2) - 2;
    bt = cur_bt + 2;
  }

  char **funcs = backtrace_symbols(bt, size);
  if unlikely (funcs == NULL) {
    ret += fputs("  (backtrace_symbols failed)", stream);
    backtrace_symbols_fd(bt, size, fileno(stream));
  } else {
    for (int j = 0; j < size; j++) {
      ret += printf("  at %s\n", funcs[j]);
    }
  }
  free(funcs);

  return ret;
#else
  return fputs("  (backtrace unavailable on non-glibc)", stream);
#endif
}


int Exception_fputs (const Exception *e, FILE *stream) {
  PROTECT_RETURN(e) 0;

  int ret = 0;

  char typename_unknown[sizeof("UnknownException(0xffffffffffffffff)")];
  const char *typename = virtual_method(Exception, e, name);
  if (typename == NULL) {
    snprintf(typename_unknown, sizeof(typename_unknown), "UnknownException(%p)", (void *) e->VTABLE(Exception));
    typename = typename_unknown;
  }
  ret += fprintf(
    stream, "Exception at file \"%s\", line %d, in %s\n",
    e->file ? e->file : "(unknown)", e->line,
    e->func ? e->func : "(unknown)"
  );
  ret += Exception_fputs_backtrace_symbols(e->bt, e->nptrs, stream);

  ret += fprintf(stream, "Printed at:\n");
  ret += Exception_fputs_backtrace_symbols(NULL, 0, stream);

  ret += fprintf(stream, "%s: ", typename);

  Exception_fputs_t e_fputs = virtual_method(Exception, e, fputs);
  if (e_fputs == NULL) {
    ret += fputs("(no fputs for exception)\n", stream);
  } else {
    ret += e_fputs((const BaseException *) e, stream);
  }

  return ret;
}


int Exception_fputs_what (const BaseException *e_, FILE *stream) {
  const Exception *e = (const Exception *) e_;

  if (e->what == NULL) {
    return fputs("(no message)\n", stream);
  } else {
    int ret = fputs(e->what, stream);
    ret += fputs("\n", stream);
    return ret;
  }
}


extern inline bool Exception_has (const Exception *e);


void Exception_dirtydestory (Exception *e) {
  PROTECT_RETURN(e);

  if (e->set == true) {
    fputs("Warning: Exception not cleaned before another coming. The previous is:\n", stderr);
  } else if (e->set == EXCEPTION_MASKED) {
    fputs("During handling of the below exception another exception occurred. The previous is:\n", stderr);
  } else {
    fputs("Error: e->set unknown. The previous is:\n", stderr);
  }
  Exception_fputs(e, stderr);
  Exception_destory(e);
}


void Exception_destory (Exception *e) {
  PROTECT_RETURN(e);

  virtual_method(Exception, e, destory)((BaseException *) e);

  e->set = false;
  e->file = NULL;
  e->line = 0;
  e->func = NULL;
  e->VTABLE(Exception) = NULL;
}


void Exception_destory_dynamic (BaseException *e_) {
  Exception *e = (Exception *) e_;

  free(e->what);
  e->what = NULL;
}


int Exception_init (Exception *e, const char *file, const char *func, unsigned line, bool no_backtrace) {
  if unlikely (e->set != false) {
    Exception_dirtydestory(e);
  }

  e->set = true;
  e->line = line;
#ifdef __GLIBC__
  if (!no_backtrace) {
    e->nptrs = backtrace(e->bt - 2, BT_BUF_SIZE + 1) - 2;
  }
#endif
  e->file = file;
  e->func = func;

  return 0;
}


/* unspecified */

extern inline int UnspecifiedException_init (
    UnspecifiedException *e, const char *file, const char *func, unsigned line,
    const char *what);


VTABLE_INIT(Exception, UnspecifiedException) = {
  .name = "UnspecifiedException",
  .fputs = Exception_fputs_what,
  .destory = Exception_destory_dynamic
};
