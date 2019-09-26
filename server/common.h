#ifndef DFCC_SERVER_COMMON_H
#define DFCC_SERVER_COMMON_H

#include <libsoup/soup.h>
#include <glib/gstdio.h>


void G_GNUC_PRINTF (4, 5) soup_xmlrpc_message_log_and_set_fault (
    SoupMessage *msg, GLogLevelFlags log_level,
    int fault_code, const gchar *format, ...);
gboolean soup_xmlrpc_message_set_response_e (SoupMessage *msg, GVariant *value);


#endif /* DFCC_SERVER_COMMON_H */
