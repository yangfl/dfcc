#include <stdarg.h>

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include <macro.h>

#include "../version.h"
#include "common.h"


void G_GNUC_PRINTF (4, 5) soup_xmlrpc_message_log_and_set_fault (
    SoupMessage *msg, GLogLevelFlags log_level,
    int fault_code, const gchar *format, ...) {
  va_list args;
  va_start(args, format);
  gchar *error_msg = g_strdup_vprintf(format, args);
  va_end(args);

  g_log(DFCC_NAME, log_level, error_msg);
  soup_xmlrpc_message_set_fault(msg, fault_code, error_msg);
  g_free(error_msg);
}


gboolean soup_xmlrpc_message_set_response_e (
    SoupMessage *msg, GVariant *value) {
  GError *error = NULL;
  gboolean ret = soup_xmlrpc_message_set_response(msg, value, &error);
  should (ret) otherwise {
    soup_xmlrpc_message_log_and_set_fault(
      msg, G_LOG_LEVEL_WARNING, 1,
      "Error when responsing XML RPC request: %s", error->message);
    g_error_free(error);
  }
  return ret;
}
