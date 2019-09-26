#include <glib.h>

#include "serverurl.h"


void ServerURL_destroy (struct ServerURL *server_url) {
  g_free(server_url->baseurl);
  g_free(server_url->username);
  g_free(server_url->password);
  g_free(server_url->proxyurl);
  g_free(server_url->server_cert);
  g_free(server_url->client_cert);
}
