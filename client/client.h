#ifndef DFCC_CLIENT_H
#define DFCC_CLIENT_H
/**
 * @defgroup Client Client
 * @{
 */

#include "config/config.h"


/**
 * @brief Start the client.
 *
 * @param config a Config
 * @return the exit status
 */
int Client_start (struct Config *config);


/**@}*/

#endif /* DFCC_CLIENT_H */
