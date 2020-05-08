#ifndef DFCC_PROTOCOL_H
#define DFCC_PROTOCOL_H

#include "./version.h"


#define DFCC_USER_AGENT DFCC_NAME "/" DFCC_VERSION

#define DFCC_COOKIES_SID "SID="

#define DFCC_RPC_PATH "/rpc"

#define DFCC_RPC_SUBMIT_METHOD_NAME "compile"
// [argv], [envp], working_directory, info -> value
#define DFCC_RPC_SUBMIT_REQUEST_SIGNATURE "(asassa{sv})"
// jid
#define DFCC_RPC_SUBMIT_RESPONSE_SIGNATURE "u"

#define DFCC_RPC_ASSOCIATE_METHOD_NAME "associate"
// path -> hash
#define DFCC_RPC_ASSOCIATE_REQUEST_SIGNATURE "a{st}"
// accepted
#define DFCC_RPC_ASSOCIATE_RESPONSE_SIGNATURE "b"

#define DFCC_RPC_QUERY_METHOD_NAME "query"
// jid nonblocking
#define DFCC_RPC_QUERY_REQUEST_SIGNATURE "(ub)"
// finished
#define DFCC_RPC_QUERY_RESPONSE_SIGNATURE "(bv)"
// missing -> hash
#define DFCC_RPC_QUERY_RESPONSE_MISSING_SIGNATURE "a{st}"
// output -> (size, hash), info -> value
#define DFCC_RPC_QUERY_RESPONSE_FINISH_SIGNATURE "(a{st}a{sv})"

#define DFCC_INFO_PATH "/info"
#define DFCC_RPC_INFO_RESPONSE_SIGNATURE "a{sv}"

#define DFCC_DOWNLOAD_PATH "/download/"

#define DFCC_UPLOAD_PATH "/upload"
// (size hash)
#define DFCC_RPC_UPLOAD_RESPONSE_SIGNATURE "(tt)"

// [missing_path]
#define DFCC_REQUEST_RPC_RESPONSE_SIGNATURE "as"


#endif /* DFCC_PROTOCOL_H */
