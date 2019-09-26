#include <glib.h>

#include <macro.h>

#include "../config.h"
#include "mux.h"


char **Config_mux (
    struct Config *config, int argc,
    const char * const argv[], const char * const envp[]) {
  gchar **args;
#ifdef G_OS_WIN32
  args = g_win32_get_command_line();
#else
  args = g_strdupv((gchar **) argv);
#endif

  // cc dfcc_client_argv -- cc_argv
  // cc cc_argv
  // dfcc dfcc_client_argv -- cc cc_argv
  // dfcc
  // dfcc dfcc_server_argv
  // dfcc cc cc_argv
  if (config->symlinked) {
    for (int i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], "--") == 0) {
        config->cc_argv = g_memdup(&args[i], (g_strv_length(&args[i]) + 1) * sizeof(gchar *));
        g_free(args[i]);
        args[i] = NULL;
        config->cc_argv[0] = g_strdup(args[0]);
        goto processed;
      }
    }

    config->cc_argv = args;
    args = g_malloc(2 * sizeof(gchar *));
    args[0] = g_strdup(config->cc_argv[0]);
    args[1] = NULL;
  } else {
    for (int i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], "--") == 0) {
        g_free(args[i]);
        args[i] = NULL;
        i++;
        config->cc_argv = g_memdup(&args[i], (g_strv_length(&args[i]) + 1) * sizeof(gchar *));
        goto processed;
      }
    }

    if (args[1] == NULL || args[1][0] == '-') {
      config->server_mode = true;
    } else {
      config->cc_argv = g_memdup(&args[1], (g_strv_length(&args[1]) + 1) * sizeof(gchar *));
      args[1] = NULL;
    }
  }

processed:

  if (!config->server_mode) {
    should (config->cc_argv != NULL) otherwise {
      g_strfreev(args);
      g_return_val_if_reached(NULL);
    }
    should (config->cc_argv[0] != NULL) otherwise {
      g_printerr("CC options required in client mode\n");
      g_strfreev(args);
      return NULL;
    }
    config->cc_envp = g_strdupv((gchar **) envp);
  }
  return args;
}
