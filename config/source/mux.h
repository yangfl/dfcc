#ifndef DFCC_CONFIG_MUX_H
#define DFCC_CONFIG_MUX_H

#include "config/config.h"


/**
 * @memberof Config
 * @private
 * @brief Set `config` with default options.
 *
 * @code{.sh}
  cc dfcc_client_argv -- cc_argv
  cc cc_argv
  dfcc dfcc_client_argv -- cc cc_argv
  dfcc
  dfcc dfcc_server_argv
  dfcc cc cc_argv
 * @endcode
 *
 * @param config a Config
 * @param argc length of `argv`
 * @param argv argument vector
 *        [array zero-terminated=0 length=argc][element-type filename]
 * @param envp environment
 *        [array zero-terminated=1][element-type filename]
 * @return the commandline arguments in the GLib filename encoding (ie: UTF-8),
 *         or NULL if error happened [transfer full]
 */
char **Config_mux (
    struct Config *config, int argc,
    const char * const argv[], const char * const envp[]);


#endif /* DFCC_CONFIG_MUX_H */
