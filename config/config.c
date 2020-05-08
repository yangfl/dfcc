#include <stddef.h>
#include <string.h>

#include <glib.h>
#include <whereami.h>

#include "common/macro.h"
#include "source/args.h"
#include "source/conffile.h"
#include "source/default.h"
#include "source/mux.h"
#include "serverurl.h"
#include "config.h"


struct StructInfo Config__info[] = {
  {"trust", G_TYPE_BOOLEAN, offsetof(struct Config, trust)},
};
const int Config__info_n = G_N_ELEMENTS(Config__info);


/**
 * @memberof Config
 * @private
 * @brief Set Config.prgpath and Config.prgdir_len.
 *
 * @param config a Config
 */
static void Config_set_prgpath (struct Config *config) {
  int prgpath_len = wai_getExecutablePath(NULL, 0, NULL);
  config->prgpath = g_malloc(prgpath_len + 1);
  wai_getExecutablePath(config->prgpath, prgpath_len,
                        (int *) &config->prgdir_len);
  config->prgpath[prgpath_len] = '\0';
}


/**
 * @memberof Config
 * @private
 * @brief Set Config.symlinked according to `argv[0]`.
 *
 * @param config a Config
 * @param arg0 `argv[0]`
 */
static void Config_set_symlinked (struct Config *config, const char *arg0) {
  gchar *prgname = g_path_get_basename(arg0);
  g_set_prgname(prgname);
  config->symlinked =
    strcmp(config->prgpath + config->prgdir_len + 1, prgname) != 0;
  g_free(prgname);
}


/**
 * @memberof Config
 * @private
 * @brief Frees associated server-part resources of a Config.
 *
 * @param config a Config
 */
static void Config_destroy_server (struct Config *config) {
  g_free(config->base_path);

  g_free(config->tls_cert_file);
  g_free(config->tls_key_file);

  g_free(config->cache_dir);
  g_free(config->hookfs);
}


/**
 * @memberof Config
 * @private
 * @brief Frees associated client-part resources of a Config.
 *
 * @param config a Config
 */
static void Config_destroy_client (struct Config *config) {
  g_strfreev(config->cc_argv);
  g_strfreev(config->cc_envp);
  g_free(config->cc_working_directory);

  if (config->server_list != NULL) {
    for (int i = 0; config->server_list[i].baseurl != NULL; i++) {
      ServerURL_destroy(&config->server_list[i]);
    }
    g_free(config->server_list);
    config->server_list = NULL;
  }
}


void Config_destroy (struct Config *config) {
  g_free(config->confpath);
  g_free(config->prgpath);

  if (config->server_mode) {
    Config_destroy_server(config);
  } else {
    Config_destroy_client(config);
  }
}


int Config_init (
    struct Config *config, int argc,
    const char * const argv[], const char * const envp[]) {
  memset(config, 0, sizeof(struct Config));

  int ret;

  Config_set_prgpath(config);
  Config_set_symlinked(config, argv[0]);

  char **args = Config_mux(config, argc, argv, envp);
  should (args != NULL) otherwise return 1;

  ret = Config_parse_args(config, args);
  should (ret == 0) otherwise return ret;

  g_strfreev(args);

  ret = Config_fill_conffile(config);
  should (ret == 0) otherwise return ret;

  return Config_fill_default(config);
}
