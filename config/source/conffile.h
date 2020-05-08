#ifndef DFCC_CONFIG_CONFFILE_H
#define DFCC_CONFIG_CONFFILE_H

#include "config/config.h"


/**
 * @memberof Config
 * @private
 * @brief Parse options from Config.confpath and set `config`.
 *
 * @param config a Config
 * @return 0 if success, otherwize nonzero
 */
int Config_fill_conffile (struct Config *config);

#endif /* DFCC_CONFIG_CONFFILE_H */
