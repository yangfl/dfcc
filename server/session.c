#include <time.h>

#include <gmodule.h>

#include "../file/remoteindex.h"

#include "session.h"


extern inline void Session_destroy (struct Session *session);
extern inline int Session_init (struct Session *session, SessionID sid);


struct SessionTable_clean_sessions_foreach_remove_ctx {
  time_t now;
  unsigned int timeout;
};


static gboolean SessionTable_clean_sessions_foreach_remove (
    gpointer key, gpointer value, gpointer user_data) {
  struct SessionTable_clean_sessions_foreach_remove_ctx *ctx =
    (struct SessionTable_clean_sessions_foreach_remove_ctx *) user_data;
  return ctx->now - ((struct Session *) value)->last_active > ctx->timeout;
}


void SessionTable_clean_sessions (GHashTable *session_table, unsigned int timeout) {
  struct SessionTable_clean_sessions_foreach_remove_ctx ctx = {
    .now = time(NULL),
    .timeout = timeout
  };
  g_hash_table_foreach_remove(
    session_table, SessionTable_clean_sessions_foreach_remove, &ctx);
}


struct Session *SessionTable_get (GHashTable *session_table, SessionID sid) {
  if (sid == SessionID__MAX) {
    return NULL;
  }
  struct Session *session = g_hash_table_lookup(session_table, &sid);
  if (session == NULL) {
    session = g_malloc(sizeof(struct Session));
    Session_init(session, sid);
    g_hash_table_insert(session_table, &session->sid, session);
  }
  return session;
}
