#include <glib.h>

#include <macro.h>

#include "../../version.h"
#include "../config.h"
#include "args.h"


static gboolean Config_parse_cc_argv (
    const gchar *option_name, const gchar *value,
    gpointer data, GError **error) {
  g_set_error(error, g_quark_from_static_string(DFCC_NAME), 0,
              "Non-option arguments not allowed before `--'");
  return FALSE;
}


int Config_parse_args (struct Config *config, char *args[]) {
  if unlikely (args == NULL || args[0] == NULL || args[1] == NULL) {
    return 0;
  }

  gchar *parameter_string = g_strdup_printf(
    "\n"
    "  %s [[CLIENT OPTION?] --] <compiler> [COMPILER OPTIONS]]\n"
    "  <compiler> [[CLIENT OPTION?] --] [COMPILER OPTIONS]\n"
    "-- distributed C/C++ compiler framework", g_get_prgname());
  GOptionContext *context = g_option_context_new(parameter_string);
  g_free(parameter_string);

  const GOptionEntry entries[] = {
    {"debug", 'd', 0, G_OPTION_ARG_NONE, &config->debug, "Enable debug output", NULL},
    {"version", 'v', 0, G_OPTION_ARG_NONE, &config->show_version, "Print the version and exit", NULL},
    {"config", 'c', 0, G_OPTION_ARG_STRING, &config->confpath, "Path to the config file", "file"},
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_CALLBACK, Config_parse_cc_argv, NULL, NULL},
    {NULL}
  };
  g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

  GOptionGroup *group_client = g_option_group_new("client", "Client Options:", "Show client help options", NULL, NULL);
  const GOptionEntry entries_client[] = {
    {"randomize", 0, 0, G_OPTION_ARG_NONE, &config->randomize, "Randomize the order of the host list before execution", NULL},
    {NULL}
  };
  g_option_group_add_entries(group_client, entries_client);
  g_option_group_set_translation_domain(group_client, GETTEXT_PACKAGE);
  g_option_context_add_group(context, group_client);

  GOptionGroup *group_server = g_option_group_new("server", "Server Options:", "Show server help options", NULL, NULL);
  const GOptionEntry entries_server[] = {
    {"foreground", 'f', 0, G_OPTION_ARG_NONE, &config->foreground, "Run the server in foreground", NULL},
    {"port", 'p', 0, G_OPTION_ARG_INT, &config->port, "Port to be listened", "N"},
    {"jobs", 'j', 0, G_OPTION_ARG_INT, &config->jobs, "Maximum jobs", "N"},
    {"base_path", 0, 0, G_OPTION_ARG_STRING, &config->base_path, "Base path", "path"},
    {"housekeeping_interval", 0, 0, G_OPTION_ARG_INT, &config->housekeeping_interval, "Housekeeping interval", "s"},
    {"session_timeout", 0, 0, G_OPTION_ARG_INT, &config->session_timeout, "Session timeout", "s"},
    {"tls_cert_file", 0, 0, G_OPTION_ARG_STRING, &config->tls_cert_file, "TLS cert file", "cert"},
    {"tls_key_file", 0, 0, G_OPTION_ARG_STRING, &config->tls_key_file, "TLS key file", "key"},
    {"cache_dir", 0, 0, G_OPTION_ARG_STRING, &config->cache_dir, "Cache dir", "dir"},
    {"no_verify_cache", 0, 0, G_OPTION_ARG_NONE, &config->no_verify_cache, "No verify cache", NULL},
    {"hookfs", 0, 0, G_OPTION_ARG_FILENAME, &config->hookfs, "Path to hookfs so", "hookfs.so"},
    {NULL}
  };
  g_option_group_add_entries(group_server, entries_server);
  g_option_group_set_translation_domain(group_server, GETTEXT_PACKAGE);
  g_option_context_add_group(context, group_server);

  GError *error = NULL;
  should (g_option_context_parse_strv(context, &args, &error)) otherwise {
    g_printerr("Option parsing failed: %s\n", error->message);
    g_option_context_free(context);
    return 1;
  }
  g_option_context_free(context);

  if (config->server_mode) {
    if (config->hookfs != NULL) {
      should (config->hookfs[0] == '\0' ||
          g_file_test(config->hookfs, G_FILE_TEST_IS_REGULAR)) otherwise {
        g_printerr("hookfs \"%s\" does not exist\n", config->hookfs);
        return 1;
      }
    }
    should (config->port < 65536) otherwise {
      g_printerr("Invalid port number %u\n", config->port);
      return 1;
    }
  } else {
    should (config->port == 0 && config->foreground == 0) otherwise {
      g_printerr("Option parsing failed: Server options in client mode\n");
      return 1;
    }
  }

  return 0;
}
