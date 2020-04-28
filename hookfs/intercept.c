#include <fcntl.h>  // open
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "macro.h"

#include "intercept.h"


int move_fd (int oldfd, int newfd) {
  should (dup2(oldfd, newfd) == 0) otherwise {
    perror("move_fd: dup2");
    return -1;
  }
  should (close(oldfd) == 0) otherwise {
    perror("move_fd: close");
    return -1;
  }
  return 0;
}


int intercept_fd (int oldfd, bool oldfd_is_write, int *saved_oldfd) {
  // create pipe
  // read = 0, write = 1
  int pipe_fds[2];
  should (pipe(pipe_fds) == 0) otherwise {
    perror("intercept_fd: pipe");
    return -1;
  }

  // save oldfd
  if (saved_oldfd != NULL) {
    *saved_oldfd = dup(oldfd);
  }

  // override oldfd
  should (move_fd(pipe_fds[oldfd_is_write ? 1 : 0], oldfd) == 0) otherwise {
    return -1;
  }

  // return the other end
  return pipe_fds[oldfd_is_write ? 0 : 1];
}


int intercept_stdin_stdout (void) {
  int middle_inner_fd;
  int inner_middle_fd;
  int outer_middle_fd;
  int middle_outer_fd;
  FILE *outer_middle;
  FILE *middle_outer;

  middle_inner_fd = intercept_fd(STDIN_FILENO, false, &outer_middle_fd);
  inner_middle_fd = intercept_fd(STDOUT_FILENO, true, &middle_outer_fd);

  outer_middle = fdopen(outer_middle_fd, "r");
  setvbuf(outer_middle, NULL, _IOLBF, BUFSIZ);
  middle_outer = fdopen(middle_outer_fd, "w");
  setvbuf(middle_outer, NULL, _IOLBF, BUFSIZ);

  return 0;
}


int pipe_stdin_stdout (const char orig_stdin_path[],
                       const char orig_stdout_path[]) {
  int stdin_pipe = open(orig_stdin_path, O_RDONLY);
  should (stdin_pipe != -1) otherwise {
    perror("open");
    return -1;
  }
  move_fd(stdin_pipe, STDIN_FILENO);

  int stdout_pipe = open(orig_stdout_path, O_WRONLY);
  should (stdout_pipe != -1) otherwise {
    perror("open");
    return -1;
  }
  move_fd(stdout_pipe, STDOUT_FILENO);

  return 0;
}
