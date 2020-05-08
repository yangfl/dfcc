#ifndef DFCC_CLIENT_REMOTE_H
#define DFCC_CLIENT_REMOTE_H
/**
 * @addtogroup Client
 * @{
 */

#include "../config/config.h"
#include "prepost.h"


/**
 * @brief Try to submit and run the compiler on one of the remote servers.
 *
 * @param config a Config
 * @return 0 if success, otherwise non-zero
 */
int Client_run_remotely (
  const struct Config *config, struct Result * restrict result,
  char * const remote_argv[], char * const remote_envp[]);


/**@}*/
#endif /* DFCC_CLIENT_REMOTE_H */
