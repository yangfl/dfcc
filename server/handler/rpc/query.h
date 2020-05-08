#ifndef DFCC_SERVER_HANDLER_RPC_QUERY_H
#define DFCC_SERVER_HANDLER_RPC_QUERY_H

#include "common.h"


/**
 * @ingroup ServerRPCHandler
 * @brief Processes XMLRPC requests of querying status of compiling jobs.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param msg a SoupMessage
 * @param param a GVariant
 */
DFCC_RPC_HANDLER(Server_rpc_query);


#endif /* DFCC_SERVER_HANDLER_RPC_QUERY_H */
