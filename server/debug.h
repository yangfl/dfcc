#ifndef DFCC_SERVER_DEBUG_H
#define DFCC_SERVER_DEBUG_H
/**
 * @addtogroup Server
 * @{
 * @defgroup ServerDebug Server Debug
 * @brief Functions for debugging server
 * @{
 */

#include <libsoup/soup.h>


/**
 * @brief Prints request for debug purposes.
 *
 * @param msg a SoupMessage containing request
 * @param path the path component of Request-URI
 */
void Server_Debug_request (SoupMessage *msg, const char *path);
/**
 * @brief Prints request and corresponding response for debug purposes.
 *
 * @param msg a SoupMessage containing request
 * @param path the path component of Request-URI
 */
void Server_Debug_request_response (SoupMessage *msg, const char *path);
/**
 * @brief Prints all listening ports of a SoupServer.
 *
 * @param server a SoupServer
 */
void Server_Debug_listening (SoupServer *server);


/**
 * @}
 * @}
 */
#endif /* DFCC_SERVER_DEBUG_H */
