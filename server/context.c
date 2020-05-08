#include <gmodule.h>

#include "config/config.h"
#include "file/cache.h"
#include "./version.h"
#include "server/job.h"
#include "server/session.h"
#include "context.h"


gboolean ServerContext__housekeep (gpointer user_data) {
  struct ServerHousekeepingContext *server_housekeeping_ctx =
    (struct ServerHousekeepingContext *) user_data;
  if unlikely (server_housekeeping_ctx->stop) {
    g_free(server_housekeeping_ctx);
    return G_SOURCE_REMOVE;
  }

  g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, "Do housekeep");

  g_rw_lock_writer_lock(&server_housekeeping_ctx->server_ctx->session_table.rwlock);
  unsigned int n_session_removed = SessionTable_clean(
    &server_housekeeping_ctx->server_ctx->session_table,
    server_housekeeping_ctx->session_timeout);
  g_rw_lock_writer_unlock(
    &server_housekeeping_ctx->server_ctx->session_table.rwlock);

  if (n_session_removed > 0) {
    JobTable_clean(&server_housekeeping_ctx->server_ctx->jobtable,
                   &server_housekeeping_ctx->server_ctx->session_table);
  }

  return G_SOURCE_CONTINUE;
}


void ServerContext_destroy (struct ServerContext *server_ctx) {
  Cache_destroy(&server_ctx->cache);
  SessionTable_destroy(&server_ctx->session_table);
  JobTable_destroy(&server_ctx->jobtable);
}


int ServerContext_init (struct ServerContext *server_ctx,
                        SoupServer *server, struct Config *config) {
  server_ctx->server = server;

  server_ctx->config = config;
  server_ctx->config->base_path_len = strlen(config->base_path);

  Cache_init(
    &server_ctx->cache, g_strdup(config->cache_dir), config->no_verify_cache);
  SessionTable_init(&server_ctx->session_table);
  JobTable_init(&server_ctx->jobtable, config->jobs);

  return 0;
}
