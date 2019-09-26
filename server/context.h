#ifndef DFCC_SERVER_CONTEXT_H
#define DFCC_SERVER_CONTEXT_H

#include <stdbool.h>

#include <libsoup/soup.h>
#include <gmodule.h>

#include "../config/config.h"
#include "../file/cache.h"
#include "job.h"


struct ServerContext {
  SoupServer *server;

  struct Config *config;
  unsigned int base_path_len;

  struct Cache cache;
  GHashTable *sessions;
  GRWLock sessions_rwlock;
  struct JobTable jobtable;
};


struct ServerHousekeepingContext {
  struct ServerContext *server_ctx;
  unsigned int session_timeout;
  bool stop;
};


gboolean ServerContext__housekeep (gpointer user_data);
void ServerContext_destroy (struct ServerContext *server_ctx);
int ServerContext_init (struct ServerContext *server_ctx,
                        SoupServer *server, struct Config *config);


#endif /* DFCC_SERVER_CONTEXT_H */
