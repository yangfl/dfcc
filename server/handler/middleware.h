#ifndef DFCC_SERVER_MIDDLEWARE_H
#define DFCC_SERVER_MIDDLEWARE_H

#include <stdbool.h>
#include <string.h>

#include <libsoup/soup.h>

#include "server/context.h"
#include "server/session.h"
#include "common.h"


/**
 * @ingroup ServerHandler
 * @brief Parse the session from the requset.
 *
 * @param server the SoupServer
 * @param msg the message being processed
 * @param path the path component of `msg`'s Request-URI
 * @param query the parsed query component of `msg`'s Request-URI
 *              [element-type utf8 utf8][allow-none]
 * @param context additional contextual information about the client
 * @param server_ctx a ServerContext
 * @param prefix_len The length of the toplevel path for the handler. -1 means
 *                   the length is 0. 0 means the path can be improper.
 * @return a Session, or NULL if not found
 */
struct Session *Server_handle_middleware (
  SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
  SoupClientContext *context, struct ServerContext *server_ctx,
  bool proper_required, int prefix_len);


#define SOUP_HANDLER_MIDDLEWARE(handler, proper_required, sid_required) \
  struct ServerContext *server_ctx G_GNUC_UNUSED = \
    (struct ServerContext *) user_data; \
  struct Session *session = Server_handle_middleware( \
    server, msg, path, query, context, user_data, \
    proper_required, strlen(SOUP_HANDLER_PATH(handler))); \
  if unlikely (sid_required && session == NULL) { \
    soup_message_set_status(msg, SOUP_STATUS_BAD_REQUEST); \
    return; \
  }


#endif /* DFCC_SERVER_MIDDLEWARE_H */
