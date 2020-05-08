#ifndef DFCC_SERVER_HANDLER_UPLOAD_H
#define DFCC_SERVER_HANDLER_UPLOAD_H

#include "common.h"


/**
 * @ingroup ServerHandler
 * @brief Processes upload (PUT) requests of source files.
 *
 * @param server the SoupServer
 * @param msg the message being processed
 * @param path the path component of `msg`'s Request-URI
 * @param query the parsed query component of `msg`'s Request-URI
 *              [element-type utf8 utf8][allow-none]
 * @param context additional contextual information about the client
 * @param user_data the data passed to `soup_server_add_handler()` or
 *                  `soup_server_add_early_handler()`.
 */
SOUP_HANDLER_PROTOTYPE(Server_handle_upload);


#endif /* DFCC_SERVER_HANDLER_UPLOAD_H */
