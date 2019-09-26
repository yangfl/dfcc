#define _GNU_SOURCE /* needed to get RTLD_NEXT defined in dlfcn.h */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <linux/limits.h>

#include "macro.h"


#define check(func) \
if (unlikely(errno)) { \
  perror(# func); \
  exit(-1); \
}

#define SOCKET_PATH "./uds_socket"

#if !DEBUG
#define DBG(...)
#else
#define DBG(...) fprintf(stderr, __VA_ARGS__)
#endif


static FILE *rx;
static FILE *tx;


static void __attribute__ ((constructor)) filetrace_init () {
  //fprintf(stderr, "arg is %s\n", getenv("FILETRACE_ARG"));
  //unsetenv("FILETRACE_ARG");

  char pidns[32]; // pid:[4026532386] = 16
  if (readlink("/proc/self/ns/pid", pidns, sizeof(pidns)) < 0) {
    perror("readlink");
    exit(-1);
  }
  strtok(pidns + 5, "]");

  int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  check(socket);

  struct sockaddr_un socket_addr = {.sun_family = AF_UNIX};
  strcpy(socket_addr.sun_path, SOCKET_PATH);
  connect(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr));
  check(connect);
  rx=fdopen(socket_fd, "rb");
  tx=fdopen(dup(socket_fd), "wb");
  setlinebuf(rx);
  setlinebuf(tx);

  char pid[7];
  sprintf(pid, "%s %d\n", pidns + 5, getpid());
  fputs(pid, tx);
}


static void __attribute__ ((destructor)) filetrace_del () {
}


static void *resolve (const char *symbol) {
  void *real = dlsym(RTLD_NEXT, symbol);
  should (real != NULL) otherwise {
    fprintf(stderr, "Error when resloving symbol %s: %s\n", symbol, dlerror());
    exit(255);
  }
  return real;
}


#define WRAP(type, symbol) \
static void *resolve_ ## symbol (void) { \
  return resolve(# symbol); \
} \
type real_ ## symbol () __attribute__ ((ifunc ("resolve_" # symbol))); \
type symbol

#define GET_PATH(path) \
char new_path[PATH_MAX]; \
if (get_rx_path(new_path, sizeof(new_path))) { \
  (path) = new_path; \
  DBG("get new path %s\n", new_path); \
}

#define DUP_RETURN(ret) \
_Pragma("GCC diagnostic push"); \
_Pragma("GCC diagnostic ignored \"-Wint-conversion\""); \
_Pragma("GCC diagnostic ignored \"-Wincompatible-pointer-types\""); \
fprintf( \
  tx, \
  _Generic((ret), \
    int: "%d\n", \
    void*: "%p\n", \
    FILE *: "%d\n", \
    DIR*: "%p\n" \
  ), \
  _Generic((ret), \
    FILE *: ((ret) ? fileno(ret) : 0), \
    default: (ret) \
  )); \
_Pragma("GCC diagnostic pop"); \
return (ret)

#define HOOK(type, func, fmt, ...) { \
  fprintf(tx, # func " " fmt "\n", ## __VA_ARGS__); \
  type ret = real_ ## func(__VA_ARGS__); \
  DUP_RETURN(ret); \
}

#define HOOK_PATH(type, func, fmt, path, ...) { \
  fprintf(tx, # func " " fmt "\n%s\n", "?", ## __VA_ARGS__, path); \
  GET_PATH(path); \
  type ret = real_ ## func((path), ## __VA_ARGS__); \
  DUP_RETURN(ret); \
}


bool get_rx_path (char new_path[], size_t size) {
  if (unlikely(fgets(new_path, size, rx) == NULL)) {
    perror("fgets");
    exit(1);
  }
  if (strcmp(new_path, ".\n") == 0) {
    return false;
  }
  strtok(new_path, "\n");
  return true;
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
HOOK(int, execve, "%s %p %p\n-", path, argv, envp)


WRAP(int, execvp) (const char *file, char *const argv[])
HOOK(int, execvp, "%s %p\n-", file, argv)


WRAP(int, access) (const char *pathname, int mode)
HOOK_PATH(int, access, "%s %d", pathname, mode)


WRAP(int, stat) (const char *pathname, struct stat *statbuf)
HOOK_PATH(int, stat, "%s %p", pathname, statbuf)


WRAP(int, lstat) (const char *pathname, struct stat *statbuf)
HOOK_PATH(int, lstat, "%s %p", pathname, statbuf)


WRAP(int, open) (const char *path, int oflag, mode_t mode)
HOOK_PATH(int, open, "%s %d %d", path, oflag, mode)


WRAP(int, open64) (const char *path, int oflag, mode_t mode)
HOOK_PATH(int, open64, "%s %d %d", path, oflag, mode)


WRAP(FILE *, fopen) (const char *filename, const char *mode)
HOOK_PATH(FILE *, fopen, "%s %s", filename, mode)


WRAP(FILE *, fopen64) (const char *filename, const char *mode)
HOOK_PATH(FILE *, fopen64, "%s %s", filename, mode)


/*
WRAP(int, fclose) (FILE *stream)
HOOK(int, fclose, "%p", stream)


WRAP(int, fclose64) (FILE *stream)
HOOK(int, fclose64, "%p", stream)
*/


WRAP(FILE *, freopen) (const char *filename, const char *mode, FILE *stream)
HOOK_PATH(FILE *, freopen, "%s %s %p", filename, mode, stream)


WRAP(FILE *, freopen64) (const char *filename, const char *mode, FILE *stream)
HOOK_PATH(FILE *, freopen64, "%s %s %p", filename, mode, stream)


WRAP(int, rename) (const char *oldname, const char *newname)
HOOK(int, rename, "%s %s", oldname, newname)


WRAP(int, unlink) (const char *filename)
HOOK(int, unlink, "%s", filename)


WRAP(int, remove) (const char *filename)
HOOK(int, remove, "%s", filename)


WRAP(FILE *, tmpfile) (void)
HOOK(FILE *, tmpfile, "")


WRAP(FILE *, tmpfile64) (void)
HOOK(FILE *, tmpfile64, "")


WRAP(DIR*, opendir) (const char *name)
HOOK_PATH(DIR*, opendir, "%s", name)


/*
WRAP(ssize_t, read) (int fd, void *buf, size_t count) {
  char pathbuf[512];
  snprintf(pathbuf, sizeof pathbuf, "/proc/self/fd/%d", fd);
  char pathname[512];
  readlink(pathbuf, pathname, sizeof pathname);
  fprintf(stderr, "reading: %d => %s count %lu\n", fd, pathname, count);
  return real_read(fd, buf, count);
}
*/
