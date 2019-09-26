#include <stdbool.h>
#include <stdint.h>

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include <macro.h>
#include <morestring.h>

#include "../config/config.h"
#include "../protocol.h"
#include "../version.h"
#include "common.h"
#include "context.h"
#include "debug.h"
#include "handler.h"
#include "session.h"
#include "server.h"


static inline bool Server_check_path (
    struct ServerContext *server_ctx,
    const char *path, unsigned int prefix_len) {
  path += server_ctx->base_path_len + prefix_len;
  if (path[0] == '/') {
    path++;
  }
  if unlikely (path[0] != '\0') {
    return false;
  }
  return true;
}


struct ServerCallbackContext {
  struct ServerContext *server_ctx;
  void (*handler) (
    struct ServerContext *, struct Session *, const char *, SoupMessage *);
  int path_len;
  bool force_sid;
};


static void Server_handle (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, gpointer data) {
  struct ServerCallbackContext *server_cb_ctx =
    (struct ServerCallbackContext *) data;

  do_once {
    if unlikely (
        server_cb_ctx->path_len != 0 && !Server_check_path(
          server_cb_ctx->server_ctx, path,
          server_cb_ctx->path_len == -1 ? 0 : server_cb_ctx->path_len)) {
      soup_message_set_status(msg, SOUP_STATUS_NOT_FOUND);
      break;
    }

    SessionID sid = SessionID__MAX;

    gchar **cookies = g_strsplit(
      soup_message_headers_get_one(msg->request_headers, "Cookie"), "; ", 0);
    for (int i = 0; cookies[i] != NULL; i++) {
      if (strscmp(cookies[i], DFCC_COOKIES_SID) == 0) {
        const char *s_sid = cookies[i] + strlen(DFCC_COOKIES_SID);
        char *s_sid_end;
        sid = strtoull(s_sid, &s_sid_end, 16);
        if (s_sid == s_sid_end) {
          sid = SessionID__MAX;
        }
        break;
      }
    }
    g_strfreev(cookies);

    if (server_cb_ctx->force_sid && sid == SessionID__MAX) {
      soup_message_set_status(msg, SOUP_STATUS_BAD_REQUEST);
      break;
    }

    server_cb_ctx->handler(
      server_cb_ctx->server_ctx,
      SessionTable_get(server_cb_ctx->server_ctx->sessions, sid), path, msg);
  }
  Server_Debug_request_response(msg, path);
}


GMainLoop *loop;


static void Server_quit (int sig) {
  g_main_loop_quit(loop);
}


int Server_start (struct Config *config) {
  // start the socket
  SoupServer *server;
  GError *error = NULL;

  do_once {
    if (config->tls_cert_file && config->tls_key_file) {
      GTlsCertificate *cert = g_tls_certificate_new_from_files(
        config->tls_cert_file, config->tls_key_file, &error);
      should (error == NULL) otherwise break;
      server = soup_server_new(
        SOUP_SERVER_SERVER_HEADER, DFCC_USER_AGENT,
        SOUP_SERVER_TLS_CERTIFICATE, cert,
        NULL);
      g_object_unref(cert);

      soup_server_listen_all(server, config->port, SOUP_SERVER_LISTEN_HTTPS,
                             &error);
      should (error == NULL) otherwise break;
    } else {
      server = soup_server_new(SOUP_SERVER_SERVER_HEADER, DFCC_USER_AGENT,
                               NULL);
      soup_server_listen_all(server, config->port, 0, &error);
      should (error == NULL) otherwise break;
    }
  }

  if (error) {
    g_printerr("Unable to create server: %s\n", error->message);
    g_error_free(error);
    return 1;
  }

  // set up context
  struct ServerContext server_ctx;
  ServerContext_init(&server_ctx, server, config);

  // set up the route
  struct ServerCallbackContext server_cb_ctx_homepage = {
    .server_ctx = &server_ctx,
    .handler = Server_handle_homepage,
    .path_len = -1
  };
  soup_server_add_handler(server, config->base_path, Server_handle,
                          &server_cb_ctx_homepage, NULL);

  char register_path[server_ctx.base_path_len +
                     MAX(
                      MAX(strlen(DFCC_RPC_PATH), strlen(DFCC_UPLOAD_PATH)),
                      MAX(strlen(DFCC_DOWNLOAD_PATH), strlen(DFCC_INFO_PATH))
                     ) + 1];
  memcpy(register_path, config->base_path, server_ctx.base_path_len);

#define register_router(prefix, path_len_, force_sid_, handler_) \
  memcpy(register_path + server_ctx.base_path_len, prefix, \
         strlen(prefix) + 1); \
  struct ServerCallbackContext server_cb_ctx_ ## prefix = { \
    .server_ctx = &server_ctx, \
    .handler = (handler_), \
    .path_len = (path_len_), \
    .force_sid = (force_sid_) \
  }; \
  soup_server_add_handler(server, register_path, Server_handle, \
                          &server_cb_ctx_ ## prefix, NULL);

  register_router(DFCC_RPC_PATH, strlen(DFCC_RPC_PATH),
                  true, Server_handle_rpc);
  register_router(DFCC_UPLOAD_PATH, strlen(DFCC_UPLOAD_PATH),
                  true, Server_handle_upload);
  register_router(DFCC_DOWNLOAD_PATH, 0, true, Server_handle_download);
  register_router(DFCC_INFO_PATH, strlen(DFCC_INFO_PATH),
                  false, Server_handle_info);

#undef register_router

  // set session cleaner
  struct ServerHousekeepingContext *server_housekeeping_ctx =
    g_malloc(sizeof(struct ServerHousekeepingContext));
  server_housekeeping_ctx->server_ctx = &server_ctx;
  server_housekeeping_ctx->session_timeout = config->session_timeout;
  server_housekeeping_ctx->stop = false;
  g_timeout_add(config->housekeeping_interval * 1000,
                ServerContext__housekeep, server_housekeeping_ctx);

  // print debug info
  Server_Debug_listening(server);
  g_log(DFCC_NAME, G_LOG_LEVEL_MESSAGE, "Waiting for requests...");

  // start
  loop = g_main_loop_new(NULL, TRUE);
  signal(SIGINT, Server_quit);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  server_housekeeping_ctx->stop = true;
  ServerContext_destroy(&server_ctx);
  return 0;
}
