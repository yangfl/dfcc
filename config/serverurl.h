#ifndef DFCC_CONFIG_SERVERURL_H
#define DFCC_CONFIG_SERVERURL_H

#include <stdbool.h>


struct ServerURL {
  char *baseurl;
  char *username;
  char *password;

  char *proxyurl;

  unsigned int timeout;

  bool use_ssl;
  bool no_strict_ssl;
  char *server_cert;
  char *client_cert;
};


void ServerURL_destroy (struct ServerURL *server_url);


#endif /* DFCC_CONFIG_SERVERURL_H */
