#ifndef HOOKFS_SOCKET_H
#define HOOKFS_SOCKET_H

#include <stddef.h>
#include <threads.h>


//! @ingroup Hookfs
struct Socket {
  mtx_t mtx;
  int fd;
};


//! @memberof Socket
ssize_t Socket_send (struct Socket *sock, const void *buf, size_t len);
//! @memberof Socket
ssize_t Socket_recv (struct Socket *sock, void *buf, size_t len);
//! @memberof Socket
void Socket_destroy (struct Socket *sock);
//! @memberof Socket
int Socket_init (struct Socket *sock, const char *path);


#endif /* HOOKFS_SOCKET_H */
