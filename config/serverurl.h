#ifndef DFCC_CONFIG_SERVERURL_H
#define DFCC_CONFIG_SERVERURL_H

#include <stdbool.h>


/**
 * @ingroup Config
 * @brief Contains information to connect to a server.
 */
struct ServerURL {
  /// Toplevel path for the server.
  char *baseurl;
  /// Server username.
  char *username;
  /// Server password.
  char *password;

  /// Proxy URL [optional]
  char *proxyurl;

  /// Maximum time allowed for responses
  unsigned int timeout;

  /// Use SSL/TlS
  bool use_ssl;
  /// Allow insecure server connections when using SSL
  bool no_strict_ssl;
  /// Server certificate file [optional]
  char *server_cert;
  /// Client certificate file [optional]
  char *client_cert;
};


/**
 * @memberof ServerURL
 * @brief Frees associated resources of a ServerURL.
 *
 * @param server_url a ServerURL
 */
void ServerURL_destroy (struct ServerURL *server_url);


#endif /* DFCC_CONFIG_SERVERURL_H */
