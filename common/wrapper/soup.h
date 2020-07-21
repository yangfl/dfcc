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

#include "gvariant.h"


void soup_xmlrpc_message_log_and_set_fault (
  SoupMessage *msg, int fault_code, const char *log_domain,
  GLogLevelFlags log_level, const char *format, ...) G_GNUC_PRINTF(5, 6) ;
gboolean soup_xmlrpc_message_set_response_e (
  SoupMessage *msg, GVariant *value, const char *log_domain);
GVariant *soup_xmlrpc_parse_response_e (
  SoupMessage *msg, const char *signature,
  const char *log_domain, GLogLevelFlags log_level);
GVariant *soup_session_xmlrpc (
  SoupSession *session, const char *uri, const char *method_name,
  GVariant *params, const char *signature, const char *log_domain,
  unsigned int *status);

#define dfcc_session_xmlrpc_variant(session, uri, method, log_domain, status, value) \
  soup_session_xmlrpc( \
    (session), (uri), DFCC_RPC_ ## method ## _METHOD_NAME, (value), \
    DFCC_RPC_ ## method ## _RESPONSE_SIGNATURE, (log_domain), (status))

#define dfcc_session_xmlrpc(session, uri, method, log_domain, status, ...) \
  dfcc_session_xmlrpc_variant( \
    (session), (uri), method, (log_domain), (status), \
    g_variant_new(DFCC_RPC_ ## method ## _REQUEST_SIGNATURE, __VA_ARGS__))


/**@}*/

#endif /* DFCC_WRAPPER_SOUP_H */
