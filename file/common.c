#include <errno.h>

#include <glib.h>

#include <macro.h>

#include "common.h"


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


GQuark DFCC_FILE_IO_ERROR = 0;


static void __attribute__((constructor)) DFCC_FILE_IO_ERROR_init (void) {
  if unlikely (DFCC_FILE_IO_ERROR == 0) {
    DFCC_FILE_IO_ERROR = g_quark_from_static_string("dfcc-file-io-error-quark");
  }
}


void dfcc_file_io_set_error (GError **error, const char *format) {
  int saved_errno = errno;
  g_set_error(error, DFCC_FILE_IO_ERROR, saved_errno,
              format, g_strerror(saved_errno));
}
