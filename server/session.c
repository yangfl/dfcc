#include <time.h>

#include <gmodule.h>

#include "../file/remoteindex.h"

#include "session.h"


extern inline bool SessionID_vaild (SessionID sid);


void Session_destroy (void *session) {
  RemoteFileIndex_destroy(&((struct Session *) session)->file_index);
}


void Session_free (void *session) {
  Session_destroy(session);
  free(session);
}


int Session_init (struct Session *session, SessionID sid) {
  session->sid = sid;
  session->last_active = time(NULL);
  RemoteFileIndex_init(&session->file_index);
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
  return session->rc == 0 && ctx->now - session->last_active > ctx->timeout;
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

  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&session_table->rwlock);
  struct Session *session = g_hash_table_lookup(session_table->table, &sid);
  g_rw_lock_reader_locker_free(locker);

  if unlikely (session == NULL) {
    session = g_malloc(sizeof(struct Session));
    Session_init(session, sid);
    g_rw_lock_writer_lock(&session_table->rwlock);
    g_hash_table_insert(session_table->table, &session->sid, session);
    g_rw_lock_writer_unlock(&session_table->rwlock);
  }

  return session;
}


void SessionTable_destroy (struct SessionTable *session_table) {
  g_hash_table_destroy(session_table->table);
  g_rw_lock_clear(&session_table->rwlock);
}


int SessionTable_init (struct SessionTable *session_table) {
  session_table->table =
    g_hash_table_new_full(g_int_hash, g_int_equal, NULL, Session_free);
  g_rw_lock_init(&session_table->rwlock);
  return 0;
}
