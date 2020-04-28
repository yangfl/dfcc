#ifndef HOOKFS_INTERCEPT_H
#define HOOKFS_INTERCEPT_H

#include <stdio.h>


int intercept_stdin_stdout (void);
int pipe_stdin_stdout (const char orig_stdin_path[],
                       const char orig_stdout_path[]);


#endif /* HOOKFS_INTERCEPT_H */
