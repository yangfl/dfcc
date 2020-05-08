#include <libsoup/soup.h>

#include "common/macro.h"
#include "common/wrapper/soup.h"
#include "../protocol.h"
#include "../debug.h"
#include "../log.h"
#include "../context.h"
#include "../session.h"
#include "middleware.h"
#include "rpc/associate.h"
#include "rpc/query.h"
#include "rpc/submit.h"
#include "rpc.h"


const char SOUP_HANDLER_PATH(Server_handle_rpc)[] = DFCC_RPC_PATH;


struct ServerRPCTable {
  const char *name;
  const char *signature;
  void (*handler) (
    struct ServerContext *, struct Session *, SoupMessage *, GVariant *);
  void *userdata;
};


static const struct ServerRPCTable rpcs[] = {
  {DFCC_RPC_SUBMIT_METHOD_NAME, DFCC_RPC_SUBMIT_REQUEST_SIGNATURE, Server_rpc_submit, NULL},
  {DFCC_RPC_ASSOCIATE_METHOD_NAME, DFCC_RPC_ASSOCIATE_REQUEST_SIGNATURE, Server_rpc_associate, NULL},
  {DFCC_RPC_QUERY_METHOD_NAME, DFCC_RPC_QUERY_REQUEST_SIGNATURE, Server_rpc_query, NULL},
};


static inline int Server__find_method_name (const char *method_name) {
  for (int i = 0; i < G_N_ELEMENTS(rpcs); i++) {
    if (strcmp(method_name, rpcs[i].name) == 0) {
      return i;
    }
  }
  return -1;
}


void Server_handle_rpc (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, gpointer user_data) {
  SOUP_HANDLER_MIDDLEWARE(Server_handle_rpc, true, true);

  // only POST allowed
  if unlikely (msg->method != SOUP_METHOD_POST) {
    soup_message_set_status(msg, SOUP_STATUS_METHOD_NOT_ALLOWED);
    return;
  }

  GError *error = NULL;
  do_once {
    SoupXMLRPCParams *xmlrpc_params;
    char *method_name = soup_xmlrpc_parse_request(
      msg->request_body->data, msg->request_body->length,
      &xmlrpc_params, &error);
    should (method_name != NULL) otherwise {
      soup_xmlrpc_message_log_and_set_fault(
        msg, 1, DFCC_SERVER_NAME, G_LOG_LEVEL_WARNING,
        "Error when parsing XMLRPC request: %s", error->message);
      break;
    }

    int method_index = Server__find_method_name(method_name);
    should (method_index != -1) otherwise {
      soup_xmlrpc_message_log_and_set_fault(
        msg, 1, DFCC_SERVER_NAME, G_LOG_LEVEL_WARNING,
        "Unknown XMLRPC request: %s", method_name);
      g_free(method_name);
      break;
    }
    g_free(method_name);

    GVariant *xmlrpc_variant = soup_xmlrpc_params_parse(
      xmlrpc_params, rpcs[method_index].signature, &error);
    soup_xmlrpc_params_free(xmlrpc_params);
    should (xmlrpc_variant != NULL) otherwise {
      soup_xmlrpc_message_log_and_set_fault(
        msg, 1, DFCC_SERVER_NAME, G_LOG_LEVEL_WARNING,
        "Error when parsing XMLRPC params: %s", error->message);
      break;
    }

    rpcs[method_index].handler(server_ctx, session, msg, xmlrpc_variant);
    Server_Debug_request_response(msg, path);
    return;
  }

  if (error != NULL) {
    g_error_free(error);
  }
}
