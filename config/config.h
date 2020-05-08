#ifndef DFCC_CONFIG_H
#define DFCC_CONFIG_H
/**
 * @defgroup Config Config
 */

#include <stdbool.h>

#include <structinfo.h>

#include "serverurl.h"


/**
 * @ingroup Config
 * @brief All information `dfcc` needed.
 */
struct Config {
  /** @name Main
   *  Parameters for both client and server
   */
  ///@{

  /// Path to the config file. Nullable.
  char *confpath;
  /// Path to the executable file of `dfcc`.
  char *prgpath;
  /// Length of the dirname of Config.prgpath.
  unsigned int prgdir_len;
  /// Whether `dfcc` is executable with another name.
  bool symlinked;
  /// Show version and exit.
  bool show_version;
  /// Run server.
  bool server_mode;
  /// Enable debug mode.
  bool debug;
  ///@}

  /** @name Server
   *  Parameters for server only
   */
  ///@{

  /// Run server in foreground. Otherwise daemonize itself after startup.
  bool foreground;
  /// Port to listen.
  unsigned int port;
  /**
   * @brief Toplevel path for all RPC handlers.
   * @sa ServerURL.baseurl
   */
  char *base_path;
  /// length of Config.base_path
  unsigned int base_path_len;

  /// Path to TLS certificate file. [optional]
  char *tls_cert_file;
  /// Path to TLS key file. [optional]
  char *tls_key_file;

  /// The number of processors configured.
  unsigned int nprocs_conf;
  /// The number of processors currently online (available).
  unsigned int nprocs_onln;

  /**
   * @brief Maximum number of possible jobs the server can handle
   *        simultaneously.
   * @sa JobTable.max_njob
   */
  unsigned int jobs;
  /// The time between housekeeping, in milliseconds (1/1000ths of a second).
  unsigned int housekeeping_interval;
  /**
   * @brief The time after which a session is considered unused.
   * @sa ServerHousekeepingContext.session_timeout
   */
  unsigned int session_timeout;

  /**
   * @brief Maximum number of possible jobs the server can handle
   *        simultaneously.
   * @sa Cache.no_verify_cache
   */
  bool no_verify_cache;
  /**
   * @brief Directory where cached remote source files are stored.
   * @sa Cache.cache_dir
   */
  char *cache_dir;

  /// Path to the preload library `hookfs`.
  char *hookfs;
  ///@}

  /** @name Client
   *  Parameters for client only
   */
  ///@{

  /// Compiler's argument vector. [array zero-terminated=1]
  char **cc_argv;
  /// Compiler's environment. [array zero-terminated=1]
  char **cc_envp;
  /// Compiler's working directory.
  char *cc_working_directory;
  /// URL and information of remote servers.
  struct ServerURL *server_list;
  /// Contact and test remote servers with random sequence.
  bool randomize;
  /// Trust server-provided source files.
  bool trust;
  ///@}
};


extern struct StructInfo Config__info[];
extern const int Config__info_n;


/**
 * @memberof Config
 * @brief Frees associated resources of a Config.
 *
 * @param config a Config
 */
void Config_destroy (struct Config *config);
/**
 * @memberof Config
 * @brief Initializes a Config with `argc`, `argv`, and `envp`.
 *
 * @param config a Config
 * @param argc length of `argv`
 * @param argv argument vector
 *        [array zero-terminated=0 length=argc][element-type filename]
 * @param envp environment
 *        [array zero-terminated=1][element-type filename]
 * @return 0 if success, otherwize nonzero
 */
int Config_init (
    struct Config *config, int argc,
    const char * const argv[], const char * const envp[]);


#endif /* DFCC_CONFIG_H */
