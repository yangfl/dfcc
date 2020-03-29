#include <gmodule.h>

#include "../config/config.h"
#include "../file/cache.h"
#include "../version.h"
#include "job.h"
#include "session.h"
#include "context.h"


gboolean ServerContext__housekeep (gpointer user_data) {
  struct ServerHousekeepingContext *server_housekeeping_ctx =
    (struct ServerHousekeepingContext *) user_data;
  if unlikely (server_housekeeping_ctx->stop) {
    g_free(server_housekeeping_ctx);
    return G_SOURCE_REMOVE;
  }

  g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, "Do housekeep");

  g_rw_lock_writer_lock(&server_housekeeping_ctx->server_ctx->sessions_rwlock);
  SessionTable_clean_sessions(
    server_housekeeping_ctx->server_ctx->sessions,
    server_housekeeping_ctx->session_timeout);
  g_rw_lock_writer_unlock(
    &server_housekeeping_ctx->server_ctx->sessions_rwlock);
  return G_SOURCE_CONTINUE;
}


void ServerContext_destroy (struct ServerContext *server_ctx) {
  Cache_destroy(&server_ctx->cache);
  g_hash_table_destroy(server_ctx->sessions);
  g_rw_lock_clear(&server_ctx->sessions_rwlock);
  JobTable_destroy(&server_ctx->jobtable);
}


int ServerContext_init (struct ServerContext *server_ctx,
                        SoupServer *server, struct Config *config) {
  server_ctx->server = server;

  server_ctx->config = config;
  server_ctx->base_path_len = strlen(config->base_path);

  Cache_init(
    &server_ctx->cache, g_strdup(config->cache_dir), config->no_verify_cache);
  server_ctx->sessions =
    g_hash_table_new_full(g_int_hash, g_int_equal, NULL, Session_free);
  g_rw_lock_init(&server_ctx->sessions_rwlock);
  JobTable_init(&server_ctx->jobtable, config->jobs);

  return 0;
}
