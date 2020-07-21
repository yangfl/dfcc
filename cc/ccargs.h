#ifndef DFCC_CCARGS_H
#define DFCC_CCARGS_H

#include <stdbool.h>

/**
 * @defgroup CC CC
 * @brief Deal with C compiler args
 * @{
 */


/**
 * @brief Tests if `cc_argv` is suitable for remote compilation.
 *
 * @param[in,out] cc_argv pointer to compiler's argument vector [array zero-terminated=1]
 * @return `true` if `cc_argv` is suitable for remote compilation
 */
bool CC_can_run_remotely (char **cc_argv[], char **cc_envp[]);


/**@}*/

#endif /* DFCC_CCARGS_H */
