#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "intercept.h"


int middle_inner_fd;
int inner_middle_fd;
int outer_middle_fd;
int middle_outer_fd;
FILE *outer_middle;
FILE *middle_outer;


int intercept_fd (int oldfd, bool oldfd_is_write, int *saved_oldfd) {
  // create pipe
  int pipe_fds[2];
  pipe(pipe_fds);

  // save oldfd
  if (saved_oldfd != NULL) {
    *saved_oldfd = dup(oldfd);
  }

  // override oldfd
  dup2(pipe_fds[oldfd_is_write ? 1 : 0], oldfd);
  // close one end of pipe (now oldfd)
  close(pipe_fds[oldfd_is_write ? 1 : 0]);

  // return the other end
  return pipe_fds[oldfd_is_write ? 0 : 1];
}


void intercept_stdin_stdout (void) {
  middle_inner_fd = intercept_fd(STDIN_FILENO, false, &outer_middle_fd);
  outer_middle = fdopen(outer_middle_fd, "r");
  setvbuf(outer_middle, NULL, _IOLBF, BUFSIZ);

  inner_middle_fd = intercept_fd(STDOUT_FILENO, true, &middle_outer_fd);
  middle_outer = fdopen(middle_outer_fd, "w");
  setvbuf(middle_outer, NULL, _IOLBF, BUFSIZ);
}
