#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <gmodule.h>

#include "macro.h"


int do_server (unsigned short port) {
  int ret = 0;

  /* Create the socket */
  int listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
  if unlikely (listen_sock == 0) {
    handle_error("socket failed");
  }
  int opt = 1;
  /* Allow socket descriptor to be reuseable */
  if unlikely (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    handle_error("setsockopt");
  }

  /* Set it up to accept connections */
  struct sockaddr_in6 server_address = {
    .sin6_family = AF_INET6,
    .sin6_addr = in6addr_any,
    .sin6_port = htons(port)
  };
  if unlikely (bind(listen_sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    handle_error("bind failed");
  }
  if unlikely (listen(listen_sock, 5) < 0) {
    handle_error("listen");
  }

  /* Initialize the set of active sockets */
  fd_set active_fd_set, read_fd_set;
  FD_ZERO(&active_fd_set);
  FD_SET(listen_sock, &active_fd_set);
  GHashTable *client_sock_buffer_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, free);
  if unlikely (client_sock_buffer_table == NULL) {
    ret = 1;
    goto destory_sock;
  }

  while (1) {
    /* Block until input arrives on one or more active sockets. */
    read_fd_set = active_fd_set;
    if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
      handle_error("select");
    }

    /* Service all the sockets with input pending. */
    for (int i = 0; i < FD_SETSIZE; i++) {
      if (FD_ISSET(i, &read_fd_set)) {
        if (i == listen_sock) {
          /* Connection request on original socket. */
          struct sockaddr_in6 client_address;
          socklen_t client_address_len = sizeof(client_address);
          int client_sock = accept(listen_sock, (struct sockaddr *) &client_address, &client_address_len);
          if unlikely (client_sock < 0) {
            handle_error("accept");
          }
          FD_SET(client_sock, &active_fd_set);
        } else {
          /* Data arriving on an already-connected socket. */
          char buf[1024];
          bzero(buf, sizeof(buf));

          int read_buf_len = read(i, buf, sizeof(buf));
          if unlikely (read_buf_len < 0) {
            handle_error("Reading stream message");
          }
          if unlikely (read_buf_len == 0) {
            fprintf(stderr, "Ending connection\n");
            break;
          }

          printf("S: %s\n", buf);
          if unlikely (write(i, buf, read_buf_len) < 0) {
            handle_error("Writing on stream socket");
          }

          close(i);
          FD_CLR(i, &active_fd_set);
        }
      }
    }
  }

destory_table:
  g_hash_table_destroy(client_sock_buffer_table);
destory_sock:
  return ret;
}
