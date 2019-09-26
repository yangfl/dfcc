#ifndef DFCC_CONFIG_H
#define DFCC_CONFIG_H

#include <stdbool.h>

#include "serverurl.h"


struct Config {
  // main
  char *confpath;
  char *prgpath;
  unsigned int prgdir_len;
  bool symlinked;
  bool show_version;
  bool server_mode;
  bool debug;

  // server
  bool foreground;
  unsigned int port;
  char *base_path;

  char *tls_cert_file;
  char *tls_key_file;

  unsigned int nprocs_conf;
  unsigned int nprocs_onln;
  unsigned int jobs;
  unsigned int housekeeping_interval;
  unsigned int session_timeout;

  bool no_verify_cache;

  char *cache_dir;
  char *hookfs;

  // client
  char **cc_argv;
  char **cc_envp;
  char *cc_working_directory;
  struct ServerURL *server_list;
  bool randomize;
};


void Config_destroy (struct Config *config);
int Config_init (
    struct Config *config, int argc,
    const char * const argv[], const char * const envp[]);


#endif /* DFCC_CONFIG_H */
