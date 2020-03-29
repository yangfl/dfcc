#ifndef DFCC_CLIENT_SESSIONID_H
#define DFCC_CLIENT_SESSIONID_H
/**
 * @addtogroup Client
 * @{
 */

#include "../server/session.h"


/**
 * @brief Get the session ID.
 *
 * The session ID is used to identify machine by remote server. Same session ID
 * for one machine can help remote server by reusing the cache.
 *
 * @return the current session ID of the machine
 */
SessionID Client__get_session_id ();


/**@}*/
#endif /* DFCC_CLIENT_SESSIONID_H */

