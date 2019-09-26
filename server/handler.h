#ifndef DFCC_SERVER_HANDLER_H
#define DFCC_SERVER_HANDLER_H

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include "context.h"
#include "session.h"


void Server_handle_rpc (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
void Server_handle_upload (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
void Server_handle_download (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
void Server_handle_info (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);
void Server_handle_homepage (
    struct ServerContext *server_ctx, struct Session *session,
    const char *path, SoupMessage *msg);


#endif /* DFCC_SERVER_HANDLER_H */
