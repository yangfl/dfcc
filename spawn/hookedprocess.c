#include <glib.h>

#include "common/macro.h"
#include "file/hash.h"
#include "log.h"
#include "hookedprocessgroup.h"
#include "process.h"
#include "hookedprocess.h"


extern inline struct HookedProcessOutput *HookedProcessOutput_new (
  gchar *path, int mode, GError **error);


/**
 * @memberof HookedProcess
 * @brief Callback when a compiler process changes its status.
 *
 * @param spawn a Process
 * @param userdata pointer to a HookedProcessGroup
 */
static void HookedProcess_onchange (void *p_, int status) {
  struct HookedProcess *p = (struct HookedProcess *) p_;
  bool mask_event = false;

  switch (status) {
    case PROCESS_STATUS_EXIT:
      p->group->manager->n_available++;
      break;
    case HOOKEDPROCESS_FILE_MISSING:
      break;
      // todo
    case HOOKEDPROCESS_OUTPUT: {
      mask_event = true;
      char *path = g_strdup(p->path);
      GError *error = NULL;
      struct HookedProcessOutput *output = HookedProcessOutput_new(path, p->mode, &error);
      should (output != NULL) otherwise {
        g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING, "Cannot create output file `%s`", path);
        g_error_free(error);
        g_free(path);
        break;
      }
      g_hash_table_insert(p->outputs, output->path, output);
      break;
    }
    default:
      g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING, "Unknown status %d", status);
  }

  if (!mask_event && p->onchange_hooked != NULL) {
    p->onchange_hooked(p, status);
  }
}


void HookedProcessOutput_destroy (struct HookedProcessOutput *output) {
  GError *error = NULL;
  should (g_close(output->fd, &error)) otherwise {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when destroying HookedProcessOutput %s: %s",
          output->tmp_path, error->message);
  }
  should (g_unlink(output->tmp_path) == 0) otherwise {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when destroying HookedProcessOutput %s: Unable to delete tmpfile",
          output->tmp_path);
  }
  g_free(output->path);
  g_free(output->tmp_path);
}


void HookedProcessOutput_free (void *output) {
  HookedProcessOutput_destroy((struct HookedProcessOutput *) output);
  g_free(output);
}


int HookedProcessOutput_init (
    struct HookedProcessOutput *output, gchar *path,
    int mode, GError **error) {
  output->fd = g_file_open_tmp(
    DFCC_SPAWN_NAME "-XXXXXX", &output->tmp_path, error);
  should (output->fd != -1) otherwise {
    return 1;
  }
  output->path = path;
  output->mode = mode;
  return 0;
}


void HookedProcess_destroy (struct HookedProcess *p) {
  Process_destroy((struct Process *) p);
  g_hash_table_destroy(p->outputs);
}


void HookedProcess_free (void *p) {
  HookedProcess_destroy(p);
  g_free(p);
}


int HookedProcess_init (
    struct HookedProcess *p, gchar **argv, gchar **envp,
    ProcessOnchangeCallback onchange, void *userdata,
    struct HookedProcessGroup *group, GError **error) {
  should (g_file_test(
      group->manager->hookfs, G_FILE_TEST_EXISTS)) otherwise {
    g_set_error(error, DFCC_SPAWN_ERROR, 0,
                "HookFs lib '%s' gone", group->manager->hookfs);
    return 1;
  }
  p->group = group;

  gchar **envp_hooked = g_strdupv(envp);
  char *ld_preload;
  if (group->manager->debug) {
    ld_preload = g_strjoin(
      ":", "libSegFault.so", group->manager->hookfs,
      g_environ_getenv(envp_hooked, "LD_PRELOAD"), NULL);
  } else {
    ld_preload = g_strjoin(
      ":", group->manager->hookfs,
      g_environ_getenv(envp_hooked, "LD_PRELOAD"), NULL);
  }
  envp_hooked = g_environ_setenv(envp_hooked, "LD_PRELOAD", ld_preload, TRUE);
  g_free(ld_preload);
  envp_hooked = g_environ_setenv(envp_hooked, "HOOKFS_NS", group->s_hgid, TRUE);
  envp_hooked = g_environ_setenv(envp_hooked, "HOOKFS_SOCK_PATH",
                                 group->manager->socket_path, TRUE);

  p->onchange_hooked = onchange;
  p->outputs = g_hash_table_new_full(
    g_str_hash, g_str_equal, NULL, HookedProcessOutput_free);

  int ret = Process_init(
    (struct Process *) p, argv, envp_hooked, group->manager->selfpath,
    HookedProcess_onchange, userdata, error);
  g_strfreev(envp_hooked);
  return_if_fail(ret == 0) ret;

  return ret;
}


struct HookedProcess *HookedProcess_new (
    gchar **argv, gchar **envp, ProcessOnchangeCallback onchange,
    void *userdata, struct HookedProcessGroup *group, GError **error) {
  struct HookedProcess *p = g_new(struct HookedProcess, 1);
  should (HookedProcess_init(
      p, argv, envp, onchange, userdata, group, error
  ) == 0) otherwise {
    g_free(p);
    return NULL;
  }
  return p;
}
