#include <time.h>

#include <gmodule.h>

#include "file/remoteindex.h"
#include "spawn/hookedprocessgroup.h"
#include "log.h"
#include "session.h"


extern inline bool SessionID_vaild (SessionID sid);
extern inline void Session_touch (struct Session *session);
extern inline void Session_disconnect (struct Session *session);
extern inline void Session_connect (struct Session *session);


void Session_destroy (struct Session *session) {
  HookedProcessGroup_destroy((struct HookedProcessGroup *) session);
}


void Session_free (void *session) {
  Session_destroy(session);
  free(session);
}


int Session_init (
    struct Session *session, SessionID sid,
    struct HookedProcessController *controller) {
  return_if_fail(HookedProcessGroup_init(
    (struct HookedProcessGroup *) session, sid, controller) == 0) 1;
  Session_touch(session);
  session->rc = 0;
  session->destructor = (void (*) (void *)) Session_destroy;
  return 0;
}


struct SessionTable_clean_foreach_remove_ctx {
  time_t now;
  unsigned int timeout;
};


static gboolean SessionTable_clean_foreach_remove_cb (
    gpointer key, gpointer value, gpointer user_data) {
  struct SessionTable_clean_foreach_remove_ctx *ctx =
    (struct SessionTable_clean_foreach_remove_ctx *) user_data;
  struct Session *session = (struct Session *) value;
  gboolean clean =
    session->rc == 0 && ctx->now - session->last_active > ctx->timeout;
  if (clean) {
    g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_DEBUG, "Clean session %x", session->hgid);
  }
  return clean;
}


unsigned int SessionTable_clean (
    struct SessionTable *session_table, unsigned int timeout) {
  struct SessionTable_clean_foreach_remove_ctx ctx = {
    .now = time(NULL),
    .timeout = timeout
  };
  return g_hash_table_foreach_remove(
    session_table->table, SessionTable_clean_foreach_remove_cb, &ctx);
}


struct Session *SessionTable_get (
    struct SessionTable *session_table, SessionID sid) {
  return_if_fail(SessionID_vaild(sid)) NULL;

  struct Session *session = (struct Session *) HookedProcessController_lookup(
    (struct HookedProcessController *) session_table, sid);
  if unlikely (session == NULL && SessionID_vaild(sid)) {
    g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_DEBUG, "Create session %x", sid);
    session = g_malloc(sizeof(struct Session));
    should (Session_init(
        session, sid, (struct HookedProcessController *) session_table
    ) == 0) otherwise {
      g_free(session);
      return NULL;
    }
    g_rw_lock_writer_lock(&session_table->rwlock);
    g_hash_table_insert(session_table->table, &session->hgid, session);
    g_rw_lock_writer_unlock(&session_table->rwlock);
  }

  return session;
}


void SessionTable_destroy (struct SessionTable *session_table) {
  HookedProcessController_destroy(
    (struct HookedProcessController *) session_table);
}


int SessionTable_init (
    struct SessionTable *session_table, unsigned int jobs,
    const char *selfpath, const char *hookfs, const char *socket_path,
    const char *cache_dir, bool no_verify_cache, GError **error) {
  return_if_fail(HookedProcessController_init(
    (struct HookedProcessController *) session_table, jobs, selfpath,
    hookfs, socket_path, cache_dir, no_verify_cache, error
  ) == 0) 1;
  return 0;
}
