#ifndef DFCC_SERVER_HANDLER_H
#define DFCC_SERVER_HANDLER_H
/**
 * @addtogroup Server
 * @{
 * @defgroup ServerHandler Server Handler
 * @brief Process client requests
 * @{
 */

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include "context.h"
#include "session.h"


/**
 * @brief Processes XMLRPC requests.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param path RPC path
 * @param msg a SoupMessage
 */
void Server_handle_rpc (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
/**
 * @brief Processes upload (PUT) requests of source files.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param path RPC path
 * @param msg a SoupMessage
 */
void Server_handle_upload (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
/**
 * @brief Processes download (PUT) requests of object files.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param path RPC path
 * @param msg a SoupMessage
 */
void Server_handle_download (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
/**
 * @brief Processes requests of infomation about server itself.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param path RPC path
 * @param msg a SoupMessage
 */
void Server_handle_info (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
/**
 * @brief Processes requests of accessing Config.base_path.
 *
 * Shows a friendly HTML page to a browser.
 *
 * @param server_ctx a ServerContext
 * @param session a Session
 * @param path RPC path
 * @param msg a SoupMessage
 */
void Server_handle_homepage (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);


/**
 * @}
 * @}
 */
#endif /* DFCC_SERVER_HANDLER_H */
