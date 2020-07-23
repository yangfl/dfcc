#ifndef DFCC_SERVER_SESSION_H
#define DFCC_SERVER_SESSION_H

#include <stdatomic.h>
#include <time.h>

#include <gmodule.h>

#include "spawn/hookedprocessgroup.h"
#include "server/_sessionid.h"


/**
 * @relates Session
 * @brief Tests if a SessionID is vaild.
 *
 * @param sid a SessionID
 * @return `true` if vaild
 */
inline bool SessionID_vaild (SessionID sid) {
  return sid != SessionID_INVAILD && sid != SessionID__MAX;
}


/**
 * @ingroup Server
 * @extends HookedProcessGroup
 * @brief Contains the information of a client identification.
 */
struct Session {
  struct HookedProcessGroup;
  /// Time when this session was last active.
  time_t last_active;
  /** @brief Reference counter giving the number of connections
   *         currently using this session.
   */
  atomic_int rc;
};


inline void Session_touch (struct Session *session) {
  session->last_active = time(NULL);
}

inline void Session_disconnect (struct Session *session) {
  session->rc--;
}

inline void Session_connect (struct Session *session) {
  session->rc++;
}

/**
 * @memberof Session
 * @brief Frees associated resources of a Session.
 *
 * @param session a Session
 */
void Session_destroy (struct Session *session);
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
int Session_init (
  struct Session *session, SessionID sid,
  struct HookedProcessGroupManager *manager);


/**
 * @ingroup Server
 * @extends HookedProcessGroupManager
 * @brief Contains the information of all Session.
 */
struct SessionManager {
  struct HookedProcessGroupManager;
};


/**
 * @memberof SessionManager
 * @brief Cleans inactive sessions.
 *
 * @param session_manager a SessionManager
 * @param timeout time after which a session is considered unused
 * @return the number of sessions removed
 */
unsigned int SessionManager_clean (
  struct SessionManager *session_manager, unsigned int timeout);
/**
 * @memberof SessionManager
 * @brief Looks up a SessionID in a SessionManager.
 *
 * If the SessionID does not exist in the SessionManager, a Session is
 * automatically created with the given `sid`.
 *
 * @param session_manager a SessionManager
 * @param sid the SessionID to look up
 * @return the associated Session [transfer-none]
 */
struct Session *SessionManager_get (
  struct SessionManager *session_manager, SessionID sid);
/**
 * @memberof SessionManager
 * @brief Frees associated resources of a SessionManager.
 *
 * @param session_manager a SessionManager
 */
void SessionManager_destroy (struct SessionManager *session_manager);
/**
 * @memberof SessionManager
 * @brief Initializes a SessionManager.
 *
 * @param session_manager a SessionManager
 * @return 0 if success, otherwize nonzero
 */
int SessionManager_init (
  struct SessionManager *session_manager, unsigned int jobs,
  const char *selfpath, const char *hookfs, const char *socket_path,
  const char *cache_dir, bool no_verify_cache, GError **error);


#endif /* DFCC_SERVER_SESSION_H */
