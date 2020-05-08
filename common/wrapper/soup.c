#include <stdarg.h>

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include <macro.h>

#include "../../version.h"
#include "soup.h"


void soup_xmlrpc_message_log_and_set_fault (
    SoupMessage *msg, GLogLevelFlags log_level,
    int fault_code, const char *format, ...) {
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
      msg, G_LOG_LEVEL_CRITICAL, 1,
      "Error when responsing XML RPC request: %s", error->message);
    g_error_free(error);
  }
  return ret;
}


GVariant *soup_xmlrpc_parse_response_e (
    SoupMessage *msg, const char *signature, GLogLevelFlags log_level) {
  GError *error = NULL;
  GVariant *response = soup_xmlrpc_parse_response(
    msg->response_body->data, msg->response_body->length,
    signature, &error);

  should (response != NULL) otherwise {
    g_log(DFCC_NAME, log_level,
          error->domain == SOUP_XMLRPC_ERROR ?
            "Error when parsing the response: %s" :
            "Error when connecting to server: %s",
          error->message);
    g_error_free(error);
  }

  return response;
}


GVariant *soup_session_xmlrpc (
    SoupSession *session, const char *uri, const char *method_name,
    GVariant *params, const char *signature, unsigned int *status) {
  GError *error = NULL;
  SoupMessage *msg = soup_xmlrpc_message_new(uri, method_name, params, &error);
  should (msg != NULL) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when building XML RPC query: %s", error->message);
    g_error_free(error);
    g_object_unref(msg);
    return NULL;
  }

  unsigned int status_ = soup_session_send_message(session, msg);
  if (status != NULL) {
    *status = status_;
  }
  should (SOUP_STATUS_IS_SUCCESSFUL(status_)) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
          "Failed to perform RPC request: %s", msg->reason_phrase);
    g_object_unref(msg);
    return NULL;
  }

  GVariant *response = soup_xmlrpc_parse_response_e(
    msg, signature, G_LOG_LEVEL_WARNING);
  g_object_unref(msg);
  return response;
}
