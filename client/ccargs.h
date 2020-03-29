#ifndef DFCC_CLIENT_CCARGS_H
#define DFCC_CLIENT_CCARGS_H

#include <stdbool.h>


/**
 * @ingroup Client
 * @brief Tests if `cc_argv` is suitable for remote compilation.
 *
 * @param[in,out] cc_argv pointer to compiler's argument vector [array zero-terminated=1]
 * @return `true` if `cc_argv` is suitable for remote compilation
 */
bool Client_can_run_remotely (char **cc_argv[]);


#endif /* DFCC_CLIENT_CCARGS_H */
