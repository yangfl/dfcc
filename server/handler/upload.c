#include "common/macro.h"
#include "common/wrapper/soup.h"
#include "../protocol.h"
#include "../log.h"
#include "middleware.h"
#include "upload.h"


const char SOUP_HANDLER_PATH(Server_handle_upload)[] = DFCC_UPLOAD_PATH;


void Server_handle_upload (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, gpointer user_data) {
  SOUP_HANDLER_MIDDLEWARE(Server_handle_upload, true, true);

  GError *error = NULL;
  struct CacheEntry *entry = Cache_index_buf(
    &server_ctx->session_manager.cache, msg->request_body->data,
    msg->request_body->length, &error);
  should (error == NULL) otherwise {
    soup_xmlrpc_message_set_fault(msg, 1, "Cannot save file: %s", error->message);
    return;
  }

  soup_xmlrpc_message_set_response_e(msg, g_variant_new(
    DFCC_RPC_UPLOAD_RESPONSE_SIGNATURE,
    msg->request_body->length, entry->hash), DFCC_SERVER_NAME);
  return;
}
