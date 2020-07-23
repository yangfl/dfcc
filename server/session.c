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
    struct HookedProcessGroupManager *manager) {
  return_if_fail(HookedProcessGroup_init(
    (struct HookedProcessGroup *) session, sid, manager) == 0) 1;
  Session_touch(session);
  session->rc = 0;
  session->destructor = (void (*) (void *)) Session_destroy;
  return 0;
}


struct SessionManager_clean_foreach_remove_ctx {
  time_t now;
  unsigned int timeout;
};


static gboolean SessionManager_clean_foreach_remove_cb (
    gpointer key, gpointer value, gpointer user_data) {
  struct SessionManager_clean_foreach_remove_ctx *ctx =
    (struct SessionManager_clean_foreach_remove_ctx *) user_data;
  struct Session *session = (struct Session *) value;
  gboolean clean =
    session->rc == 0 && ctx->now - session->last_active > ctx->timeout;
  if (clean) {
    g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_DEBUG, "Clean session %x", session->hgid);
  }
  return clean;
}


unsigned int SessionManager_clean (
    struct SessionManager *session_manager, unsigned int timeout) {
  struct SessionManager_clean_foreach_remove_ctx ctx = {
    .now = time(NULL),
    .timeout = timeout
  };
  return g_hash_table_foreach_remove(
    session_manager->table, SessionManager_clean_foreach_remove_cb, &ctx);
}


struct Session *SessionManager_get (
    struct SessionManager *session_manager, SessionID sid) {
  return_if_fail(SessionID_vaild(sid)) NULL;

  struct Session *session = (struct Session *) HookedProcessGroupManager_lookup(
    (struct HookedProcessGroupManager *) session_manager, sid);
  if unlikely (session == NULL && SessionID_vaild(sid)) {
    g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_DEBUG, "Create session %x", sid);
    session = g_malloc(sizeof(struct Session));
    should (Session_init(
        session, sid, (struct HookedProcessGroupManager *) session_manager
    ) == 0) otherwise {
      g_free(session);
      return NULL;
    }
    g_rw_lock_writer_lock(&session_manager->rwlock);
    g_hash_table_insert(session_manager->table, &session->hgid, session);
    g_rw_lock_writer_unlock(&session_manager->rwlock);
  }

  return session;
}


void SessionManager_destroy (struct SessionManager *session_manager) {
  HookedProcessGroupManager_destroy(
    (struct HookedProcessGroupManager *) session_manager);
}


int SessionManager_init (
    struct SessionManager *session_manager, unsigned int jobs,
    const char *selfpath, const char *hookfs, const char *socket_path,
    const char *cache_dir, bool no_verify_cache, GError **error) {
  return_if_fail(HookedProcessGroupManager_init(
    (struct HookedProcessGroupManager *) session_manager, jobs, selfpath,
    hookfs, socket_path, cache_dir, no_verify_cache, error
  ) == 0) 1;
  return 0;
}
