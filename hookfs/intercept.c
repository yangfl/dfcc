#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "intercept.h"


int intercepted_stdin_fd;
int intercepted_stdout_fd;
int real_stdin_fd;
int real_stdout_fd;
FILE *real_stdin;
FILE *real_stdout;


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
  intercepted_stdin_fd = intercept_fd(STDIN_FILENO, false, &real_stdin_fd);
  real_stdin = stdin;
  stdin = fdopen(STDOUT_FILENO, "r");
  setvbuf(stdin, NULL, _IOLBF, BUFSIZ);

  intercepted_stdout_fd = intercept_fd(STDOUT_FILENO, true, &real_stdout_fd);
  real_stdout = stdout;
  stdout = fdopen(STDOUT_FILENO, "w");
  setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
}
