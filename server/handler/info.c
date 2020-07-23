#include "common/macro.h"
#include "common/wrapper/soup.h"
#include "../protocol.h"
#include "../log.h"
#include "middleware.h"
#include "upload.h"


const char SOUP_HANDLER_PATH(Server_handle_info)[] = DFCC_INFO_PATH;


void Server_handle_info (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, gpointer user_data) {
  SOUP_HANDLER_MIDDLEWARE(Server_handle_info, false, false);

  GVariantBuilder builder;
  g_variant_builder_init(&builder,
                         G_VARIANT_TYPE(DFCC_RPC_INFO_RESPONSE_SIGNATURE));
  g_variant_builder_add(&builder, "{sv}", "Server",
                        g_variant_new_string(DFCC_USER_AGENT));
  g_variant_builder_add(&builder, "{sv}", "Nproc-configured",
                        g_variant_new_int32(server_ctx->config->nprocs_conf));
  g_variant_builder_add(&builder, "{sv}", "Nproc-online",
                        g_variant_new_int32(server_ctx->config->nprocs_onln));
  g_variant_builder_add(&builder, "{sv}", "Jobs",
                        g_variant_new_int32(server_ctx->config->jobs));
  g_variant_builder_add(&builder, "{sv}", "Current-jobs", g_variant_new_int32(
    server_ctx->config->jobs - server_ctx->session_manager.n_available));
  soup_xmlrpc_message_set_response_e(
    msg, g_variant_builder_end(&builder), DFCC_SERVER_NAME);
}
