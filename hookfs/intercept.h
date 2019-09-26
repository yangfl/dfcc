#ifndef HOOKFS_INTERCEPT_H
#define HOOKFS_INTERCEPT_H

#include <stdio.h>


extern int intercepted_stdin_fd;
extern int intercepted_stdout_fd;
extern int real_stdin_fd;
extern int real_stdout_fd;
extern FILE *real_stdin;
extern FILE *real_stdout;


void intercept_stdin_stdout (void);


#endif /* HOOKFS_INTERCEPT_H */
