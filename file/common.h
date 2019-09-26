#ifndef DFCC_FILE_COMMON_H
#define DFCC_FILE_COMMON_H

#include <unistd.h>

#include <glib.h>
#include <glib/gstdio.h>


extern GQuark DFCC_FILE_IO_ERROR;


void dfcc_file_io_set_error (GError **error, const char *format);

#define WRAP_IO_GERROR(type, func, params, args, test, msg) \
inline type func ## _e params { \
  type ret = func args; \
  should (ret test) otherwise { \
    dfcc_file_io_set_error(error, msg ": %s"); \
  } \
  return ret; \
}

WRAP_IO_GERROR(
  int, g_stat, (const gchar *filename, GStatBuf *buf, GError **error),
  (filename, buf), == 0, "Failed to stat file")
WRAP_IO_GERROR(
  gint, g_mkdir_with_parents, (
    const gchar *pathname, gint mode, GError **error),
  (pathname, mode), == 0, "Failed to mkdir")
WRAP_IO_GERROR(
  ssize_t, read, (int fd, void *buf, size_t count, GError **error),
  (fd, buf, count), >= 0, "Failed to read")
WRAP_IO_GERROR(
  ssize_t, write, (int fd, const void *buf, size_t count, GError **error),
  (fd, buf, count), >= 0, "Failed to write")
WRAP_IO_GERROR(
  FILE *, g_fopen, (
    const gchar *filename, const gchar *mode, GError **error),
  (filename, mode), != NULL, "Failed to fopen")
WRAP_IO_GERROR(
  size_t, fwrite, (
    const void *ptr, size_t size, size_t nmemb, FILE *stream, GError **error),
  (ptr, size, nmemb, stream), == 0, "Failed to fwrite")


#endif /* DFCC_FILE_COMMON_H */
