#ifndef DFCC_CONFIG_DEFAULT_H
#define DFCC_CONFIG_DEFAULT_H

#include "../config.h"


/**
 * @memberof Config
 * @private
 * @brief Set `config` with default options.
 *
 * @param config a Config
 * @return 0 if success, otherwize nonzero
 */
int Config_fill_default (struct Config *config);

#endif /* DFCC_CONFIG_DEFAULT_H */
