#ifndef DFCC_CONFIG_ARGS_H
#define DFCC_CONFIG_ARGS_H

#include "../config.h"


/**
 * @memberof Config
 * @private
 * @brief Parse command line options and set `config`.
 *
 * @param config a Config
 * @param args argument vector [array zero-terminated=1]
 * @return 0 if success, otherwize nonzero
 */
int Config_parse_args (struct Config *config, char *args[]);


#endif /* DFCC_CONFIG_ARGS_H */
