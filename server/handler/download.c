#include "common/macro.h"
#include "common/wrapper/soup.h"
#include "../protocol.h"
#include "middleware.h"
#include "download.h"


const char SOUP_HANDLER_PATH(Server_handle_download)[] = DFCC_DOWNLOAD_PATH;


void Server_handle_download (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, gpointer user_data) {
  SOUP_HANDLER_MIDDLEWARE(Server_handle_download, false, true);

  do_once {
    const char *s_token =
      path + server_ctx->config->base_path_len +
      strlen(SOUP_HANDLER_PATH(Server_handle_download));
    FileHash hash = FileHash_from_string(s_token);
    break_if_fail(hash != 0);
    //g_hash_table_lookup(server_ctx->jobs, &hash);
  }

  soup_xmlrpc_message_set_fault(msg, 1, "err");
}
