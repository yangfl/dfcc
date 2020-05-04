#ifndef DFCC_CLIENT_LOCAL_H
#define DFCC_CLIENT_LOCAL_H

#include "../config/config.h"
#include "prepost.h"


/**
 * @ingroup Client
 * @brief Run the compiler locally.
 *
 * @param config a Config
 * @return the exit status of the compiler
 */
int Client_run_locally (struct Config *config, struct Result * restrict result);


#endif /* DFCC_CLIENT_LOCAL_H */
