#ifndef DFCC_PROTOCOL_H
#define DFCC_PROTOCOL_H

#include "version.h"


#define DFCC_PORT 0xdfc

#define DFCC_USER_AGENT DFCC_NAME "/" DFCC_VERSION

#define DFCC_COOKIES_SID "sid="

#define DFCC_RPC_PATH "/rpc"

#define DFCC_RPC_COMPILE_METHOD_NAME "compile"
// [argv] [envp] working_directory
#define DFCC_RPC_COMPILE_REQUEST_SIGNATURE "(asass)"
// jid
#define DFCC_RPC_COMPILE_RESPONSE_SIGNATURE "u"

#define DFCC_RPC_MAP_METHOD_NAME "map"
// path -> (size, hash)
#define DFCC_RPC_MAP_REQUEST_SIGNATURE "(a{s(tt)})"
// path -> accepted
#define DFCC_RPC_MAP_RESPONSE_SIGNATURE "a{sb}"

#define DFCC_RPC_QUERY_METHOD_NAME "query"
// jid nonblocking
#define DFCC_RPC_QUERY_REQUEST_SIGNATURE "(ub)"
// finished [finished ? (output -> (size, hash)) : (missing -> (size, hash))]
#define DFCC_RPC_QUERY_RESPONSE_SIGNATURE "(ba{s(tt)})"

#define DFCC_INFO_PATH "/info"
#define DFCC_RPC_INFO_RESPONSE_SIGNATURE "a{sv}"

#define DFCC_DOWNLOAD_PATH "/download/"

#define DFCC_UPLOAD_PATH "/upload"
// (size hash)
#define DFCC_RPC_UPLOAD_RESPONSE_SIGNATURE "(tt)"

// [missing_path]
#define DFCC_REQUEST_RPC_RESPONSE_SIGNATURE "as"


#endif /* DFCC_PROTOCOL_H */
