#ifndef DFCC_SERVER_CONTEXT_H
#define DFCC_SERVER_CONTEXT_H

#include <stdatomic.h>
#include <stdbool.h>

#include <libsoup/soup.h>

#include "config/config.h"
#include "file/cache.h"
#include "server/session.h"


/**
 * @ingroup Server
 * @brief Contains information about the server
 */
struct ServerContext {
  /// SoupServer instance.
  SoupServer *server;
  /// Config for the server.
  struct Config *config;
  /// Session table.
  struct SessionTable session_table;
};


/**
 * @ingroup Server
 * @brief Userdata for ServerContext__housekeep.
 *
 * @sa ServerContext
 */
struct ServerHousekeepingContext {
  /// Pointer to a ServerContext.
  struct ServerContext *server_ctx;
  /// The time after which a session is considered unused.
  unsigned int session_timeout;
  /// Whether the server has stopped.
  bool stop;
};


/**
 * @memberof ServerHousekeepingContext
 * @brief Do server housekeeping, by cleaning unused sessions.
 *
 * For use in `g_timeout_add`.
 *
 * @param server_ctx a ServerContext
 * @return `G_SOURCE_CONTINUE` if housekeeping should be continued.
 */
gboolean ServerContext__housekeep (gpointer user_data);
/**
 * @memberof ServerContext
 * @brief Frees associated resources of a ServerContext.
 *
 * @param server_ctx a ServerContext
 */
void ServerContext_destroy (struct ServerContext *server_ctx);
/**
 * @memberof ServerContext
 * @brief Initializes a ServerContext with SoupServer instance and Config.
 *
 * @param server_ctx a ServerContext
 * @param server a SoupServer
 * @param config a Config
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int ServerContext_init (
  struct ServerContext *server_ctx, SoupServer *server,
  struct Config *config, GError **error);


#endif /* DFCC_SERVER_CONTEXT_H */
