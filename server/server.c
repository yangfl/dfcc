/**
 * @addtogroup Server
 * @{
 */

#include <stdint.h>

#include <libsoup/soup.h>
#include <glib/gstdio.h>

#include "common/macro.h"
#include "config/config.h"
#include "log.h"
#include "handler/handler.h"
#include "protocol.h"
#include "context.h"
#include "debug.h"
#include "server.h"


GMainLoop *loop;


/**
 * @brief Callback when receives `SIGINT` signal.
 *
 * Quit loop.
 *
 * @param sig signal
 */
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
    } else {
      server = soup_server_new(SOUP_SERVER_SERVER_HEADER, DFCC_USER_AGENT,
                               NULL);
      soup_server_listen_all(server, config->port, 0, &error);
    }
    break_if_fail(error == NULL);
  }

  if (error) {
    g_printerr("Unable to create server: %s\n", error->message);
    g_error_free(error);
    return 1;
  }

  // set up context
  struct ServerContext server_ctx;
  should (ServerContext_init(
      &server_ctx, server, config, &error) == 0) otherwise {
    g_printerr("Unable to initialise server context: %s\n", error->message);
    g_error_free(error);
    return 2;
  }

  // set up the route
  ADD_HANDLER(Server_handle_homepage);
  ADD_HANDLER(Server_handle_rpc);
  ADD_HANDLER(Server_handle_upload);
  ADD_HANDLER(Server_handle_download);
  ADD_HANDLER(Server_handle_info);

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
  g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_MESSAGE, "Waiting for requests...");

  // start
  loop = g_main_loop_new(NULL, TRUE);
  signal(SIGINT, Server_quit);
  g_main_loop_run(loop);

  g_main_loop_unref(loop);
  server_housekeeping_ctx->stop = true;
  ServerContext_destroy(&server_ctx);
  soup_server_disconnect(server);
  g_object_unref(server);
  return 0;
}


/**@}*/
