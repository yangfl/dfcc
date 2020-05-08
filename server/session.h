#ifndef DFCC_SERVER_SESSION_H
#define DFCC_SERVER_SESSION_H

#include <time.h>

#include <glib.h>
#include <gmodule.h>

#include "file/remoteindex.h"


/**
 * @memberof Session
 * @brief Session ID
 */
typedef uint32_t SessionID;
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
  /// Compiler jobs.
  //struct JobTable jobtable;
};


/**
 * @memberof Session
 * @brief Frees associated resources of a Session.
 *
 * @param session a Session
 */
void Session_destroy (void *session);
/**
 * @memberof Session
 * @brief Frees a Session and associated resources.
 *
 * @param session a Session
 */
void Session_free (void *session);
/**
 * @memberof Session
 * @brief Initializes a Session with Session ID.
 *
 * @param session a Session
 * @param sid Session ID
 * @return 0 if success, otherwize nonzero
 */
int Session_init (struct Session *session, SessionID sid);


/**
 * @ingroup Server
 * @brief Contains the information of all sessions.
 *
 * @sa Session
 */
struct SessionTable {
  /// Hash table mapping SessionID to Session.
  GHashTable *table;
  /// Lock for `table`.
  GRWLock rwlock;
};


/**
 * @memberof SessionTable
 * @brief Cleans inactive sessions.
 *
 * @param session_table a SessionTable
 * @param timeout time after which a session is considered unused
 * @return the number of sessions removed
 */
unsigned int SessionTable_clean (
  struct SessionTable *session_table, unsigned int timeout);
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
struct Session *SessionTable_get (
  struct SessionTable *session_table, SessionID sid);
/**
 * @memberof SessionTable
 * @brief Frees associated resources of a SessionTable.
 *
 * @param session_table a SessionTable
 */
void SessionTable_destroy (struct SessionTable *session_table);
/**
 * @memberof SessionTable
 * @brief Initializes a SessionTable.
 *
 * @param session_table a SessionTable
 * @return 0 if success, otherwize nonzero
 */
int SessionTable_init (struct SessionTable *session_table);


#endif /* DFCC_SERVER_SESSION_H */
