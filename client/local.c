#include <glib.h>

#include "common/macro.h"
#include "config/config.h"
#include "spawn/subprocess.h"
#include "./version.h"
#include "local.h"


int Client_run_locally (
    struct Config *config, struct Result * restrict result) {
  GError *error = NULL;
  int ret = Subprocess_init(
    NULL, config->cc_argv, config->cc_envp, config->prgpath, NULL, NULL, &error);
  should (error == NULL) otherwise {
    if (error->domain != G_SPAWN_EXIT_ERROR) {
      g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL, error->message);
    }
    g_error_free(error);
  }
  return ret;
}
