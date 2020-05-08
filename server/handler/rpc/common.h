#ifndef DFCC_SERVER_HANDLER_RPC_COMMON_H
#define DFCC_SERVER_HANDLER_RPC_COMMON_H
/**
 * @ingroup ServerHandler
 * @defgroup ServerRPCHandler Server RPC Handler
 * @brief Process client RPC requests
 * @{
 */

#include <libsoup/soup.h>

#include "server/context.h"
#include "server/session.h"


#define DFCC_RPC_HANDLER(handler) \
  void handler ( \
    struct ServerContext *server_ctx, struct Session *session, \
    SoupMessage *msg, GVariant *param)


/**@}*/

#endif /* DFCC_SERVER_HANDLER_RPC_COMMON_H */
