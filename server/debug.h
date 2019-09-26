#ifndef DFCC_SERVER_DEBUG_H
#define DFCC_SERVER_DEBUG_H

#include <libsoup/soup.h>


void Server_Debug_request (SoupMessage *msg, const char *path);
void Server_Debug_request_response (SoupMessage *msg, const char *path);
void Server_Debug_listening (SoupServer *server);


#endif /* DFCC_SERVER_DEBUG_H */
