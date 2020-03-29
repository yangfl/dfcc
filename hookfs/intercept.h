#ifndef HOOKFS_INTERCEPT_H
#define HOOKFS_INTERCEPT_H

#include <stdio.h>


extern int middle_inner_fd;
extern int inner_middle_fd;
extern int outer_middle_fd;
extern int middle_outer_fd;
extern FILE *outer_middle;
extern FILE *middle_outer;


void intercept_stdin_stdout (void);


#endif /* HOOKFS_INTERCEPT_H */
