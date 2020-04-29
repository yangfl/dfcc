#ifndef HOOKFS_SOCKET_H
#define HOOKFS_SOCKET_H

#include <stddef.h>


struct Socket {
  int fd;
};


ssize_t Socket_send (struct Socket *sock, const void *buf, size_t len);
ssize_t Socket_recv (struct Socket *sock, void *buf, size_t len);
void Socket_destroy (struct Socket *sock);
int Socket_init (struct Socket *sock, const char *path);


#endif /* HOOKFS_SOCKET_H */
