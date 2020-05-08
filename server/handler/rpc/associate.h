#ifndef DFCC_SERVER_HANDLER_RPC_ASSOCIATE_H
#define DFCC_SERVER_HANDLER_RPC_ASSOCIATE_H

#include "common.h"


/**
 * @ingroup ServerRPCHandler
 * @brief Processes XMLRPC requests of mapping a file path to a hash.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param msg a SoupMessage
 * @param param a GVariant
 */
DFCC_RPC_HANDLER(Server_rpc_associate);


#endif /* DFCC_SERVER_HANDLER_RPC_ASSOCIATE_H */
