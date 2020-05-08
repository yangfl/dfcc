#include <gmodule.h>

#include "config/config.h"
#include "log.h"
#include "session.h"
#include "context.h"


gboolean ServerContext__housekeep (gpointer user_data) {
  struct ServerHousekeepingContext *server_housekeeping_ctx =
    (struct ServerHousekeepingContext *) user_data;
  if unlikely (server_housekeeping_ctx->stop) {
    g_free(server_housekeeping_ctx);
    return G_SOURCE_REMOVE;
  }

  g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_DEBUG, "Do housekeep");

  g_rw_lock_writer_lock(&server_housekeeping_ctx->server_ctx->session_table.rwlock);
  SessionTable_clean(
    &server_housekeeping_ctx->server_ctx->session_table,
    server_housekeeping_ctx->session_timeout);
  g_rw_lock_writer_unlock(
    &server_housekeeping_ctx->server_ctx->session_table.rwlock);

  return G_SOURCE_CONTINUE;
}


void ServerContext_destroy (struct ServerContext *server_ctx) {
  SessionTable_destroy(&server_ctx->session_table);
}


int ServerContext_init (
    struct ServerContext *server_ctx, SoupServer *server,
    struct Config *config, GError **error) {
  return_if_fail(SessionTable_init(
    &server_ctx->session_table, config->jobs, config->prgpath, config->hookfs,
    HOOKFS_SOCKET_PATH, config->cache_dir, config->no_verify_cache, error
  ) == 0) 1;
  server_ctx->server = server;
  server_ctx->config = config;
  return 0;
}
