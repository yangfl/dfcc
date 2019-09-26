#ifndef DFCC_CONFIG_MUX_H
#define DFCC_CONFIG_MUX_H

#include "../config.h"


char **Config_mux (
    struct Config *config, int argc,
    const char * const argv[], const char * const envp[]);


#endif /* DFCC_CONFIG_MUX_H */
