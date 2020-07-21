#include <libsoup/soup.h>

#include "common/macro.h"
#include "common/wrapper/gvariant.h"
#include "common/wrapper/threads.h"
#include "common/wrapper/soup.h"
#include "../../protocol.h"
#include "../../log.h"
#include "query.h"


struct QueryCallbackContext {
  struct ServerContext *server_ctx;
  struct Session *session;
  SoupMessage *msg;
};


/**
 * @brief Processes XMLRPC requests of nonblockingly querying status of
 *        compiling jobs.
 */
static void Server_rpc_query_response (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, struct HookedProcess *p) {
  GVariant *filelist;
  if (p->stopped) {
    GVariant *outputs;
    GVariant *info;
    filelist = g_variant_new_boolean(p->stopped);
  } else {
    filelist = g_variant_new_boolean(p->stopped);
  }
  soup_xmlrpc_message_set_response_e(msg, g_variant_new(
    DFCC_RPC_QUERY_RESPONSE_SIGNATURE, p->stopped, filelist), DFCC_SERVER_NAME);
}


static void Server_rpc_query_callback (struct HookedProcess *p) {
  struct QueryCallbackContext *cb_ctx =
    (struct QueryCallbackContext *) p->userdata;
  Server_rpc_query_response(
    cb_ctx->server_ctx, cb_ctx->session, cb_ctx->msg, p);
  soup_server_unpause_message(cb_ctx->server_ctx->server, cb_ctx->msg);
}


void Server_rpc_query (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, GVariant *param) {
  GPid pid;
  gboolean nonblocking;
  g_variant_get(param, DFCC_RPC_QUERY_REQUEST_SIGNATURE, &pid, &nonblocking);
  struct HookedProcess *p = HookedProcessGroup_lookup(
    (struct HookedProcessGroup *) session, pid);
  should (p != NULL) otherwise {
    soup_message_set_status(msg, SOUP_STATUS_NOT_FOUND);
    soup_xmlrpc_message_set_fault(msg, 0, "JID not found");
    return;
  }

  GError *error = NULL;
  should (mtx_lock_e(&p->mtx, &error) == thrd_success) otherwise {
    g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_ERROR,
          "%s: %s", __func__, error->message);
    g_error_free(error);
  }
  if (p->stopped || nonblocking) {
    Server_rpc_query_response(server_ctx, session, msg, p);
  } else {
    struct QueryCallbackContext *cb_ctx = g_new(struct QueryCallbackContext, 1);
    cb_ctx->server_ctx = server_ctx;
    cb_ctx->session = session;
    cb_ctx->msg = msg;
    p->userdata = cb_ctx;
    p->onexit_hooked = Server_rpc_query_callback;
    soup_server_pause_message(server_ctx->server, msg);
  }
  if likely (error == NULL) {
    should (mtx_unlock_e(&p->mtx, &error) == thrd_success) otherwise {
      g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_ERROR,
            "%s: %s", __func__, error->message);
      g_error_free(error);
    }
  }

  g_variant_unref(param);
}
