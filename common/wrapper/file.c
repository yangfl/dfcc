#include <glib.h>

#include "file.h"


extern inline int g_stat_e (
    const gchar *filename, GStatBuf *buf, GError **error);
extern inline gint g_mkdir_with_parents_e (
    const gchar *pathname, gint mode, GError **error);
extern inline ssize_t read_e (int fd, void *buf, size_t count, GError **error);
extern inline ssize_t write_e (
    int fd, const void *buf, size_t count, GError **error);
extern inline FILE *g_fopen_e (
    const gchar *filename, const gchar *mode, GError **error);
extern inline size_t fwrite_e (
    const void *ptr, size_t size, size_t nmemb, FILE *stream, GError **error);


ssize_t readfd (int fd, char *path, size_t maxsize) {
  char pathbuf[512];
  snprintf(pathbuf, sizeof(pathbuf), "/proc/self/fd/%d", fd);
  return readlink(pathbuf, path, maxsize);
}
