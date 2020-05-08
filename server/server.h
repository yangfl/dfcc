#ifndef DFCC_SERVER_H
#define DFCC_SERVER_H
/**
 * @defgroup Server Server
 * @brief Server related code
 * @{
 */

#include "config/config.h"


/**
 * @brief Start the server loop.
 *
 * The funtion never returns until the program receives a `SIGINT` signal
 * (`Ctrl-C`).
 *
 * @param config a Config
 * @return the exit status
 */
int Server_start (struct Config *config);


/**@}*/

#endif /* DFCC_SERVER_H */
