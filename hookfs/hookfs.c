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
#include "hack.h"

#include "intercept.h"

/**
 * @defgroup Hookfs Hookfs
 * @{
 */


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
 * @brief Resolve the real function provided by libc.
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
 * @brief Wrap a libc function.
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
 * @brief Read the wrapped path from controller.
 *
 * @param[out] new_path wrapped path
 * @param size maximum length of `new_path`
 * @return `true` if `new_path` vaild
 */
static bool get_wrapped_path (char new_path[], size_t size) {
  should (fgets(new_path, size, outer_middle) != NULL) otherwise {
    perror("fgets");
    exit(1);
  }
  size_t new_path_len = strlen(new_path);
  should (new_path[new_path_len - 1] == '\n') otherwise {
    fprintf(stderr, "get_wrapped_path: Path too long\n");
    exit(1);
  }
  new_path[new_path_len - 1] = '\0';
  if (strcmp(new_path, ".") == 0) {
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

#define HOOK(type, func, s_i_path, fmt, ...) { \
  fputs(# func, middle_outer); \
  fprintf(middle_outer, " %d %s\n" fmt, VA_NARGS(__VA_ARGS__), s_i_path, ## __VA_ARGS__); \
  fflush(middle_outer); \
  type ret = libc_ ## func(__VA_ARGS__); \
  return ret; \
}

#define HOOK_PATH(type, func, s_i_path, path, fmt, ...) { \
  fputs(# func, middle_outer); \
  fprintf(middle_outer, " %d %s?\n" fmt, VA_NARGS(__VA_ARGS__), s_i_path, ## __VA_ARGS__); \
  fflush(middle_outer); \
  GET_PATH(path); \
  type ret = libc_ ## func(__VA_ARGS__); \
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
HOOK(int, execve, "0", "%s\n%p\n%p\n", path, argv, envp)


WRAP(int, execvp) (const char *file, char *const argv[])
HOOK(int, execvp, "0", "%s\n%p\n", file, argv)


WRAP(int, access) (const char *pathname, int mode)
HOOK_PATH(int, access, "0", pathname, "%s\n%d\n", pathname, mode)


WRAP(int, stat) (const char *pathname, struct stat *statbuf)
HOOK_PATH(int, stat, "0", pathname, "%s\n%p\n", pathname, statbuf)


WRAP(int, lstat) (const char *pathname, struct stat *statbuf)
HOOK_PATH(int, lstat, "0", pathname, "%s\n%p\n", pathname, statbuf)


WRAP(int, open) (const char *path, int oflag, mode_t mode)
HOOK_PATH(int, open, "0", path, "%s\n%d\n%d\n", path, oflag, mode)


WRAP(int, open64) (const char *path, int oflag, mode_t mode)
HOOK_PATH(int, open64, "0", path, "%s\n%d\n%d\n", path, oflag, mode)


WRAP(FILE *, fopen) (const char *filename, const char *mode)
HOOK_PATH(FILE *, fopen, "0", filename, "%s\n%s\n", filename, mode)


WRAP(FILE *, fopen64) (const char *filename, const char *mode)
HOOK_PATH(FILE *, fopen64, "0", filename, "%s\n%s\n", filename, mode)


/*
WRAP(int, fclose) (FILE *stream)
HOOK(int, fclose, "0", "%p\n", stream)


WRAP(int, fclose64) (FILE *stream)
HOOK(int, fclose64, "0", "%p\n", stream)
*/


WRAP(FILE *, freopen) (const char *filename, const char *mode, FILE *stream)
HOOK_PATH(FILE *, freopen, "0", filename, "%s\n%s\n%p\n", filename, mode, stream)


WRAP(FILE *, freopen64) (const char *filename, const char *mode, FILE *stream)
HOOK_PATH(FILE *, freopen64, "0", filename, "%s\n%s\n%p\n", filename, mode, stream)


WRAP(int, rename) (const char *oldname, const char *newname)
HOOK(int, rename, "0 1", "%s\n%s\n", oldname, newname)


WRAP(int, unlink) (const char *filename)
HOOK(int, unlink, "0", "%s\n", filename)


WRAP(int, remove) (const char *filename)
HOOK(int, remove, "0", "%s\n", filename)


WRAP(FILE *, tmpfile) (void)
HOOK(FILE *, tmpfile, "", "")


WRAP(FILE *, tmpfile64) (void)
HOOK(FILE *, tmpfile64, "", "")


WRAP(DIR*, opendir) (const char *name)
HOOK_PATH(DIR*, opendir, "0", name, "%s\n", name)


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
  intercept_stdin_stdout();
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
}


/**@}*/
