#ifndef DFCC_SERVER_SESSION_H
#define DFCC_SERVER_SESSION_H

#include <time.h>

#include <glib.h>
#include <gmodule.h>

#include <class.h>

#include "../file/remoteindex.h"


typedef guint32 SessionID;
#define SessionID__MAX UINT32_MAX


struct Session {
  // Unique ID for this session.
  SessionID sid;

  // Reference counter giving the number of connections
  // currently using this session.
  unsigned int rc;

  // Time when this session was last active.
  time_t last_active;

  // Files of the remote client
  struct RemoteFileIndex file_index;
};


inline void Session_destroy (struct Session *session) {
  RemoteFileIndex_destroy(&session->file_index);
}

GENERATE_FREE_FUNC(Session, g_free);

inline int Session_init (struct Session *session, SessionID sid) {
  session->sid = sid;
  session->last_active = time(NULL);
  RemoteFileIndex_init(&session->file_index);
  return 0;
}


void SessionTable_clean_sessions (GHashTable *session_table, unsigned int timeout);
struct Session *SessionTable_get (GHashTable *session_table, SessionID sid);
#define SessionTable_free g_hash_table_destroy
#define SessionTable_new() \
  g_hash_table_new_full(g_int_hash, g_int_equal, NULL, Session_free)


#endif /* DFCC_SERVER_SESSION_H */
