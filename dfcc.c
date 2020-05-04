#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <glib.h>

#include <macro.h>

#include "client/client.h"
#include "server/server.h"
#include "config/config.h"
#include "version.h"


extern char **environ;


static void show_version () {
  printf("%s %s\n", DFCC_NAME, DFCC_VERSION);
}


int main (int argc, char *argv[]) {
  if (g_getenv(DFCC_LOOP_DETECTION_ENV) != NULL) {
    g_printerr("Recursive call detected!\n");
    return 255;
  }

  int ret;

  struct Config config;
  should (Config_init(
    &config, argc, (const char **) argv, (const char **) environ) == 0
  ) otherwise return 255;

  if (config.show_version) {
    show_version();
    return EXIT_SUCCESS;
  }

  if (config.debug) {
    g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
  }

  if (config.server_mode) {
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, "Server mode");
    if (config.debug == config.foreground) {
      should (daemon(1, 0) == 0) otherwise {
        g_printerr("Daemonization failed\n");
        return EXIT_FAILURE;
      }
    }
    ret = Server_start(&config);
  } else {
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, "Client mode");
    ret = Client_start(&config);
  }

  Config_destroy(&config);
  return ret == 0 ? EXIT_SUCCESS : ret;
}
