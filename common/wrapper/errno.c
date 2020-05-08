#include <errno.h>

#include <glib.h>

#include "errno.h"


void g_set_error_errno (GError **error, GQuark domain, const char *format) {
  int saved_errno = errno;
  g_set_error(error, domain, saved_errno, format, g_strerror(saved_errno));
}
