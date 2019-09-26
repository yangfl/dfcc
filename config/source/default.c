#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "../../protocol.h"
#include "../../version.h"
#include "../config.h"
#include "default.h"


static int Config_fill_default_server (struct Config *config) {
  if (config->hookfs == NULL) {
    static const char exe_dir[] = "";
    static const char * const hookfs_search_paths[] = {
#ifdef HOOKFS_SEARCH_PATH
      HOOKFS_SEARCH_PATH,
#endif
      "/usr/lib/dfcc/" DFCC_HOOKFS_FILENAME, exe_dir, "./" DFCC_HOOKFS_FILENAME,
    };

    for (int i = 0; i < G_N_ELEMENTS(hookfs_search_paths); i++) {
      if (hookfs_search_paths[i] == exe_dir) {
        char dirsep = config->prgpath[config->prgdir_len];
        config->prgpath[config->prgdir_len] = '\0';

        char *prgdir_search_path =
          g_build_path(config->prgpath, DFCC_HOOKFS_FILENAME, NULL);
        config->prgpath[config->prgdir_len] = dirsep;
        if (g_file_test(prgdir_search_path, G_FILE_TEST_IS_REGULAR)) {
          config->hookfs = prgdir_search_path;
          goto hookfs_found;
        } else {
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
                "\"%s\" does not exist", prgdir_search_path);
          g_free(prgdir_search_path);
        }
      } else {
        if (g_file_test(hookfs_search_paths[i], G_FILE_TEST_IS_REGULAR)) {
          config->hookfs = g_strdup(hookfs_search_paths[i]);
          goto hookfs_found;
        } else {
          g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
                "\"%s\" does not exist", hookfs_search_paths[i]);
        }
      }
    }

    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL, "Cannot find " DFCC_HOOKFS_FILENAME);
    return 1;

    hookfs_found:;
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, "Use hookfs at \"%s\"", config->hookfs);
  }

  if (config->port == 0) {
    config->port = DFCC_PORT;
  }
  if (config->base_path == NULL) {
    config->base_path = g_strdup("");
  }

  int nprocs_conf = sysconf(_SC_NPROCESSORS_CONF);
  if (config->nprocs_conf == 0 || config->nprocs_conf > nprocs_conf) {
    config->nprocs_conf = nprocs_conf;
  }
  int nprocs_onln = sysconf(_SC_NPROCESSORS_ONLN);
  if (config->nprocs_onln == 0 || config->nprocs_onln > nprocs_onln) {
    config->nprocs_onln = nprocs_onln;
  }
  if (config->jobs == 0) {
    config->jobs = config->nprocs_onln;
  }
  if (config->housekeeping_interval == 0) {
    config->housekeeping_interval = 60 * 1;
  }
  if (config->session_timeout == 0) {
    config->session_timeout = 60 * 2;
  }

  if (config->cache_dir == NULL) {
    config->cache_dir = g_strdup("~/.cache/dfcc");
  }

  return 0;
}


static int Config_fill_default_client (struct Config *config) {
  if (config->cc_working_directory == NULL) {
    config->cc_working_directory = g_get_current_dir();
  }

  // temp
  if (config->server_list == NULL) {
    config->server_list = g_malloc(2 * sizeof(struct ServerURL));
    config->server_list[0].baseurl = g_strdup("http://127.0.0.1:3580/");
    config->server_list[1].baseurl = NULL;
    return 0;
  }

  return 0;
}


int Config_fill_default (struct Config *config) {
  if (config->server_mode) {
    return Config_fill_default_server(config);
  } else {
    return Config_fill_default_client(config);
  }
}
