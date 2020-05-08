#ifndef DFCC_SERVER_HANDLER_RPC_SUBMIT_H
#define DFCC_SERVER_HANDLER_RPC_SUBMIT_H

#include "common.h"


/**
 * @ingroup ServerRPCHandler
 * @brief Processes XMLRPC requests of submitting a new compiling job.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param msg a SoupMessage
 * @param param a GVariant
 */
DFCC_RPC_HANDLER(Server_rpc_submit);


#endif /* DFCC_SERVER_HANDLER_RPC_SUBMIT_H */
