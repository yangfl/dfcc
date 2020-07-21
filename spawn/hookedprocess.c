#include <glib.h>

#include "common/macro.h"
#include "file/hash.h"
#include "log.h"
#include "hookedprocessgroup.h"
#include "process.h"
#include "hookedprocess.h"


extern inline struct HookedProcessOutput *HookedProcessOutput_new (
    gchar *path, int mode, GError **error);


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
    HookedProcessExitCallback onexit, void *userdata,
    struct HookedProcessGroup *group, GError **error) {
  should (g_file_test(
      group->controller->hookfs, G_FILE_TEST_EXISTS)) otherwise {
    g_set_error(error, DFCC_SPAWN_ERROR, 0,
                "HookFs lib '%s' gone", group->controller->hookfs);
    return 1;
  }
  p->group = group;

  gchar **envp_hooked = g_strdupv(envp);
  char *ld_preload;
  if (group->controller->debug) {
    ld_preload = g_strjoin(
      ":", "libSegFault.so", group->controller->hookfs,
      g_environ_getenv(envp_hooked, "LD_PRELOAD"), NULL);
  } else {
    ld_preload = g_strjoin(
      ":", group->controller->hookfs,
      g_environ_getenv(envp_hooked, "LD_PRELOAD"), NULL);
  }
  envp_hooked = g_environ_setenv(envp_hooked, "LD_PRELOAD", ld_preload, TRUE);
  g_free(ld_preload);
  envp_hooked = g_environ_setenv(envp_hooked, "HOOKFS_NS", group->s_hgid, TRUE);
  envp_hooked = g_environ_setenv(envp_hooked, "HOOKFS_SOCK_PATH",
                                 group->controller->socket_path, TRUE);

  p->onexit_hooked = onexit;
  p->outputs = g_hash_table_new_full(
    g_str_hash, g_str_equal, NULL, HookedProcessOutput_free);

  int ret = Process_init(
    (struct Process *) p, argv, envp_hooked, group->controller->selfpath,
    HookedProcessGroup_onexit, userdata, error);
  g_strfreev(envp_hooked);
  return_if_fail(ret == 0) ret;

  return ret;
}


struct HookedProcess *HookedProcess_new (
    gchar **argv, gchar **envp, HookedProcessExitCallback onexit,
    void *userdata, struct HookedProcessGroup *group, GError **error) {
  struct HookedProcess *p = g_new(struct HookedProcess, 1);
  should (HookedProcess_init(
      p, argv, envp, onexit, userdata, group, error
  ) == 0) otherwise {
    g_free(p);
    return NULL;
  }
  return p;
}
