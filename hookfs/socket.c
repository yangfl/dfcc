#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <threads.h>
#include <unistd.h>

#include "common/macro.h"
#include "socket.h"


ssize_t Socket_send (struct Socket *sock, const void *buf, size_t len) {
  ssize_t ret = send(sock->fd, buf, len, 0);
  should (ret >= 0) otherwise {
    perror("send");
  }
  return ret;
}


ssize_t Socket_recv (struct Socket *sock, void *buf, size_t len) {
  ssize_t ret = recv(sock->fd, buf, len, 0);
  should (ret >= 0) otherwise {
    perror("recv");
  }
  return ret;
}


void Socket_destroy (struct Socket *sock) {
  close(sock->fd);
  mtx_destroy(&sock->mtx);
}


int Socket_init (struct Socket *sock, const char *path) {
  int ret;

  should (mtx_init(&sock->mtx, mtx_plain) == thrd_success) otherwise {
    perror("mtx_init");
    return 1;
  }

  sock->fd = socket(PF_UNIX, SOCK_STREAM, 0);
  should (sock->fd >= 0) otherwise {
    perror("sock");
    return 1;
  }

  do_once {
    struct sockaddr_un addr = {
      .sun_family = AF_UNIX,
    };
    size_t path_len = strlen(path);
    addr.sun_path[0] = '\0';
    memcpy(addr.sun_path + 1, path, path_len);
    should (connect(sock->fd, (struct sockaddr *) &addr,
        sizeof(addr.sun_family) + 1 + path_len) == 0) otherwise {
      perror("connect");
      ret = 1;
      break;
    }

    return 0;
  }

  close(sock->fd);
  return ret;
}
