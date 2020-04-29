#define _GNU_SOURCE /* needed to get RTLD_NEXT defined in dlfcn.h */
#include <alloca.h>  // alloca
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>  // RTLD_NEXT
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <linux/limits.h>  // PATH_MAX

#include "macro.h"

#include "intercept.h"
#include "serializer.h"

/**
 * @defgroup Hookfs Hookfs
 * @{
 */


int outer_middle_fd;
int middle_outer_fd;
FILE *outer_middle;
FILE *middle_outer;
struct SerializerIOFuncs iofuncs;

#define check(func) \
should (errno == 0) otherwise { \
  perror(# func); \
  exit(-1); \
}

#if !DEBUG
#define DBG(...)
#else
#define DBG(...) fprintf(stderr, __VA_ARGS__)
#endif


/**
 * @brief Resolves the real function provided by libc.
 *
 * @param symbol string of function name
 * @return libc function `symbol`
 */
static void *resolve (const char *symbol) {
  void *real = dlsym(RTLD_NEXT, symbol);
  should (real != NULL) otherwise {
    fprintf(stderr, "Error when resloving symbol %s: %s\n", symbol, dlerror());
    exit(255);
  }
  return real;
}


/**
 * @brief Wraps a libc function.
 *
 * @code{.c}
  WRAP(int, open) (const char *path, int oflag, mode_t mode) {
    ...
    int real_ret = libc_open(path, oflag, mode);
    ...
  }
 * @endcode
 *
 * @param type the return type of function `symbol`
 * @param symbol a libc function
 */
#define WRAP(type, symbol) \
  static void *resolve_ ## symbol (void) { \
    return resolve(# symbol); \
  } \
  type libc_ ## symbol () __attribute__ ((ifunc ("resolve_" # symbol))); \
  type symbol


/**
 * @brief Reads the wrapped path from controller.
 *
 * @param[out] new_path wrapped path
 * @param size maximum length of `new_path`
 * @return `true` if `new_path` vaild
 */
static bool get_wrapped_path (char new_path[], size_t size) {
  should (deserialize_string(&iofuncs, new_path, size, NULL) != NULL) otherwise {
    fprintf(stderr, "get_wrapped_path: Error\n");
    exit(1);
  }
  getc(outer_middle);  // '\n'
  if (new_path[0] == '\0') {
    return false;
  }
  return true;
}


/**
 * @brief Get and point `path` to the wrapped path.
 *
 * @param path
 */
#define GET_PATH(path) { \
  char *new_path = alloca(PATH_MAX); \
  if (get_wrapped_path(new_path, sizeof(new_path))) { \
    (path) = new_path; \
  } \
}

#define HOOK(type, func, args, ...) { \
  serialize_string(&iofuncs, # func); \
  __VA_ARGS__ \
  serialize_end(&iofuncs); \
  type ret = libc_ ## func args; \
  return ret; \
}

#define HOOK_PATH(type, func, args, path, ...) { \
  char buf[Hookfs_MAX_TOKEN_LEN]; \
  iofuncs.ostream = buf;
  serialize_string(&iofuncs, # func); \
  __VA_ARGS__ \
  serialize_end(&iofuncs); \
  GET_PATH(path); \
  type ret = libc_ ## func args; \
  return ret; \
}


WRAP(int, execl) (const char *path, const char *arg0, ...) {
  va_list ap;

  int argc = 2;
  va_start(ap, arg0);
  while (va_arg(ap, char*))
    argc++;
  va_end(ap);

  const char *argv[argc];
  argv[0] = arg0;
  va_start(ap, arg0);
  for (int i = 1; i < argc; i++)
    argv[i] = va_arg(ap, char*);
  va_end(ap);
  return execve(path, (char *const*)argv, environ);
}


WRAP(int, execle) (const char *path, const char *arg0, .../*, char *const envp[]*/) {
  va_list ap;

  int argc = 2;
  va_start(ap, arg0);
  while (va_arg(ap, char*))
    argc++;
  va_end(ap);

  const char *argv[argc];
  argv[0] = arg0;
  va_start(ap, arg0);
  for (int i = 1; i < argc; i++)
    argv[i] = va_arg(ap, char*);
  char *const* envp = va_arg(ap, char *const*);
  va_end(ap);
  return execve(path, (char *const*)argv, envp);
}


WRAP(int, execlp) (const char *file, const char *arg0, ...) {
  va_list ap;

  int argc = 2;
  va_start(ap, arg0);
  while (va_arg(ap, char*))
    argc++;
  va_end(ap);

  const char *argv[argc];
  argv[0] = arg0;
  va_start(ap, arg0);
  for (int i = 1; i < argc; i++)
    argv[i] = va_arg(ap, char*);
  va_end(ap);
  return execvp(file, (char *const*)argv);
}


WRAP(int, execv) (const char *path, char *const argv[]) {
  return execve(path, argv, environ);
}


WRAP(int, execve) (const char *path, char *const argv[], char *const envp[])
HOOK(int, execve, (path, argv, envp),
  serialize_string(&iofuncs, path);
  serialize_strv(&iofuncs, argv);
  serialize_strv(&iofuncs, envp);
)


WRAP(int, execvp) (const char *file, char *const argv[])
HOOK(int, execvp, (file, argv),
  serialize_string(&iofuncs, file);
  serialize_strv(&iofuncs, argv);
)


WRAP(int, access) (const char *pathname, int mode)
HOOK_PATH(int, access, (pathname, mode), pathname,
  serialize_string(&iofuncs, pathname);
  serialize_printf(&iofuncs, "%d", mode);
)


WRAP(int, stat) (const char *pathname, struct stat *statbuf)
HOOK_PATH(int, stat, (pathname, statbuf), pathname,
  serialize_string(&iofuncs, pathname);
  serialize_printf(&iofuncs, "%p", statbuf);
)


WRAP(int, lstat) (const char *pathname, struct stat *statbuf)
HOOK_PATH(int, lstat, (pathname, statbuf), pathname,
  serialize_string(&iofuncs, pathname);
  serialize_printf(&iofuncs, "%p", statbuf);
)


/*
WRAP(int, open) (const char *path, int oflag, mode_t mode)
HOOK_PATH(int, open, (path, oflag, mode), path,
  serialize_string(&iofuncs, path);
  serialize_printf(&iofuncs, "%d", oflag);
  serialize_printf(&iofuncs, "%d", mode);
)


WRAP(int, open64) (const char *path, int oflag, mode_t mode)
HOOK_PATH(int, open64, (path, oflag, mode), path,
  serialize_string(&iofuncs, path);
  serialize_printf(&iofuncs, "%d", oflag);
  serialize_printf(&iofuncs, "%d", mode);
)
*/


WRAP(FILE *, fopen) (const char *filename, const char *mode)
HOOK_PATH(FILE *, fopen, (filename, mode), filename,
  serialize_string(&iofuncs, filename);
  serialize_string(&iofuncs, mode);
)


WRAP(FILE *, fopen64) (const char *filename, const char *mode)
HOOK_PATH(FILE *, fopen64, (filename, mode), filename,
  serialize_string(&iofuncs, filename);
  serialize_string(&iofuncs, mode);
)


/*
WRAP(int, fclose) (FILE *stream)
HOOK(int, fclose, "0", "%p\n", stream)


WRAP(int, fclose64) (FILE *stream)
HOOK(int, fclose64, "0", "%p\n", stream)
*/


WRAP(FILE *, freopen) (const char *filename, const char *mode, FILE *stream)
HOOK_PATH(FILE *, freopen, (filename, mode, stream), filename,
  serialize_string(&iofuncs, filename);
  serialize_string(&iofuncs, mode);
  serialize_printf(&iofuncs, "%p", stream);
)


WRAP(FILE *, freopen64) (const char *filename, const char *mode, FILE *stream)
HOOK_PATH(FILE *, freopen64, (filename, mode, stream), filename,
  serialize_string(&iofuncs, filename);
  serialize_string(&iofuncs, mode);
  serialize_printf(&iofuncs, "%p", stream);
)


WRAP(int, rename) (const char *oldname, const char *newname)
HOOK(int, rename, (oldname, newname),
  serialize_string(&iofuncs, oldname);
  serialize_string(&iofuncs, newname);
)


WRAP(int, unlink) (const char *filename)
HOOK(int, unlink, (filename),
  serialize_string(&iofuncs, filename);
)


WRAP(int, remove) (const char *filename)
HOOK(int, remove, (filename),
  serialize_string(&iofuncs, filename);
)


WRAP(FILE *, tmpfile) (void)
HOOK(FILE *, tmpfile, ())


WRAP(FILE *, tmpfile64) (void)
HOOK(FILE *, tmpfile64, ())


WRAP(DIR *, opendir) (const char *name)
HOOK_PATH(DIR *, opendir, (name), name,
  serialize_string(&iofuncs, name);
)


/*
WRAP(ssize_t, read) (int fd, void *buf, size_t count) {
  char pathbuf[512];
  snprintf(pathbuf, sizeof pathbuf, "/proc/self/fd/%d", fd);
  char pathname[512];
  readlink(pathbuf, pathname, sizeof pathname);
  fprintf(stderr, "reading: %d => %s count %lu\n", fd, pathname, count);
  return libc_read(fd, buf, count);
}
*/


static void __attribute__ ((destructor)) hookfs_del () {

}


static void __attribute__ ((constructor)) hookfs_init () {
  iofuncs.ostream = middle_outer;
  iofuncs.printf = (SerializerIOFuncs__printf_t) fprintf;
  iofuncs.write = (SerializerIOFuncs__write_t) fwrite;
  iofuncs.istream = outer_middle;
  iofuncs.scanf = (SerializerIOFuncs__scanf_t) fscanf;
  iofuncs.read = (SerializerIOFuncs__read_t) fread;

  char orig_stdin_path[PATH_MAX];
  should (get_wrapped_path(orig_stdin_path, sizeof(orig_stdin_path))) otherwise {
    fputs("stdin path unprovided\n", stderr);
    exit(-1);
  }

  char orig_stdout_path[PATH_MAX];
  should (get_wrapped_path(orig_stdout_path, sizeof(orig_stdout_path))) otherwise {
    fputs("stdout path unprovided\n", stderr);
    exit(-1);
  }

  pipe_stdin_stdout(orig_stdin_path, orig_stdout_path);
}


/**@}*/
