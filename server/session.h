#ifndef DFCC_SERVER_SESSION_H
#define DFCC_SERVER_SESSION_H

#include <time.h>

#include <glib.h>
#include <gmodule.h>

#include <class.h>

#include "../file/remoteindex.h"


/**
 * @memberof Session
 * @brief Session ID
 */
typedef guint32 SessionID;
/**
 * @memberof Session
 * @static
 * @brief Maximum possible Session ID
 */
#define SessionID__MAX UINT32_MAX


/**
 * @relates Session
 * @brief Tests if a SessionID is vaild.
 *
 * @param sid a SessionID
 * @return `true` if vaild
 */
inline bool SessionID_vaild (SessionID sid) {
  return sid != 0 && sid != SessionID__MAX;
}



/**
 * @ingroup Server
 * @brief Contains the information of a client identification.
 */
struct Session {
  /// Unique ID for this session.
  SessionID sid;

  /** @brief Reference counter giving the number of connections
   *         currently using this session.
   */
  unsigned int rc;

  /// Time when this session was last active.
  time_t last_active;

  /// Files of the remote client.
  struct RemoteFileIndex file_index;
};


/**
 * @memberof Session
 * @brief Frees associated resources of a Session.
 *
 * @param session a Session
 */
inline void Session_destroy (struct Session *session) {
  RemoteFileIndex_destroy(&session->file_index);
}

/**
 * @memberof Session
 * @brief Frees a Session and associated resources.
 *
 * @param session a Session
 */
inline void Session_free (struct Session *session) {
  Session_destroy(session);
  free(session);
}

/**
 * @memberof Session
 * @brief Initializes a Session with Session ID.
 *
 * @param session a Session
 * @param sid Session ID
 * @return 0 if success, otherwize nonzero
 */
inline int Session_init (struct Session *session, SessionID sid) {
  session->sid = sid;
  session->last_active = time(NULL);
  RemoteFileIndex_init(&session->file_index);
  return 0;
}


/**
 * @ingroup Server
 * @struct SessionTable
 * @extends GHashTable
 */


/**
 * @memberof SessionTable
 * @brief Cleans unused sessions.
 *
 * @param session_table a SessionTable
 * @param timeout time after which a session is considered unused
 */
void SessionTable_clean_sessions (GHashTable *session_table, unsigned int timeout);
/**
 * @memberof SessionTable
 * @brief Looks up a SessionID in a SessionTable.
 *
 * If the SessionID does not exist in the SessionTable, a Session is
 * automatically created with the given `sid`.
 *
 * @param session_table a SessionTable
 * @param sid the SessionID to look up
 * @return the associated Session [transfer-none]
 */
struct Session *SessionTable_get (GHashTable *session_table, SessionID sid);
/**
 * @fn void SessionTable_free (GHashTable *session_table)
 * @memberof SessionTable
 * @brief Frees a SessionTable and associated resources.
 *
 * @param session_table a SessionTable
 */
#define SessionTable_free g_hash_table_destroy
/**
 * @fn void SessionTable_new ()
 * @memberof SessionTable
 * @brief Creates a new SessionTable.
 *
 * @return a new SessionTable
 */
#define SessionTable_new() \
  g_hash_table_new_full(g_int_hash, g_int_equal, NULL, Session_free)


#endif /* DFCC_SERVER_SESSION_H */
