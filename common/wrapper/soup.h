#ifndef DFCC_WRAPPER_SOUP_H
#define DFCC_WRAPPER_SOUP_H
/**
 * @ingroup Wrapper
 * @defgroup Soup Soup
 * @brief libsoup-related wrapper functions
 * @{
 */

#include <libsoup/soup.h>
#include <glib/gstdio.h>


void G_GNUC_PRINTF (4, 5) soup_xmlrpc_message_log_and_set_fault (
  SoupMessage *msg, GLogLevelFlags log_level,
  int fault_code, const char *format, ...);
gboolean soup_xmlrpc_message_set_response_e (SoupMessage *msg, GVariant *value);
GVariant *soup_xmlrpc_parse_response_e (
  SoupMessage *msg, const char *signature, GLogLevelFlags log_level);
GVariant *soup_session_xmlrpc (
  SoupSession *session, const char *uri, const char *method_name,
  GVariant *params, const char *signature);

#define dfcc_session_xmlrpc_variant(session, uri, method, value) \
  soup_session_xmlrpc( \
    (session), (uri), DFCC_RPC_ ## method ## _METHOD_NAME, (value), \
    DFCC_RPC_ ## method ## _RESPONSE_SIGNATURE)

#define dfcc_session_xmlrpc(session, uri, method, args...) \
  dfcc_session_xmlrpc_variant( \
    (session), (uri), method, \
    g_variant_new(DFCC_RPC_ ## method ## _REQUEST_SIGNATURE, args))

#define return_if_g_variant_not_type(v, s) \
  should (g_variant_is_of_type((v), G_VARIANT_TYPE(s))) otherwise \
    if (g_log(DFCC_NAME, G_LOG_LEVEL_WARNING, \
          "Expect type '%s', got '%s'", (s), g_variant_get_type_string(v)), 1) \
      return

/**@}*/

#endif /* DFCC_WRAPPER_SOUP_H */
