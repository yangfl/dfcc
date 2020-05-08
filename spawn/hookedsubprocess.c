#include <glib.h>

#include "common/macro.h"
#include "file/hash.h"
#include "./version.h"
#include "spawn/subprocess.h"
#include "hookedsubprocess.h"


extern inline struct HookedSubprocessOutput *HookedSubprocessOutput_new (
    gchar *path, int mode, GError **error);


void HookedSubprocessOutput_destroy (struct HookedSubprocessOutput *output) {
  GError *error = NULL;
  should (g_close(output->fd, &error)) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when destroying HookedSubprocessOutput %s: %s",
          output->tmp_path, error->message);
  }
  should (g_unlink(output->tmp_path) == 0) otherwise {
    g_log(DFCC_NAME, G_LOG_LEVEL_CRITICAL,
          "Error when destroying HookedSubprocessOutput %s: Unable to delete tmpfile",
          output->tmp_path);
  }
  g_free(output->path);
  g_free(output->tmp_path);
}


void HookedSubprocessOutput_free (void *output) {
  HookedSubprocessOutput_destroy((struct HookedSubprocessOutput *) output);
  g_free(output);
}


int HookedSubprocessOutput_init (
    struct HookedSubprocessOutput *output, gchar *path,
    int mode, GError **error) {
  output->fd = g_file_open_tmp(DFCC_NAME "-XXXXXX", &output->tmp_path, error);
  should (output->fd != -1) otherwise {
    return 1;
  }
  output->path = path;
  output->mode = mode;
  return 0;
}


static GThreadPool *HookedSubprocess__threadpool = NULL;


static int HookedSubprocess_poll (
    struct HookedSubprocess *p, unsigned short *revents) {
  GPollFD fds[] = {
    {p->stderr, G_IO_IN | G_IO_HUP | G_IO_ERR},
  };
  int ret = g_poll(fds, 1, -1);
  if (revents != NULL) {
    *revents = fds[0].revents;
  }
  return ret;
}


void HookedSubprocess_join (struct HookedSubprocess *p) {
  if (!p->stopped) {
    unsigned short revents;
    HookedSubprocess_poll(p, &revents);
    if unlikely (revents & G_IO_ERR || revents & G_IO_HUP) {
      return;
    }
  }
}


gboolean HookedSubprocess_run (struct HookedSubprocess *p, GError **error) {
  return g_thread_pool_push(HookedSubprocess__threadpool, p, error);
}


void HookedSubprocess_destroy (struct HookedSubprocess *p) {
  Subprocess_destroy((struct Subprocess *) p);

  g_hash_table_destroy(p->outputs);
}


int HookedSubprocess_init (
    struct HookedSubprocess *p, gchar **argv, gchar **envp,
    const char *hookfs, const char *selfpath,
    struct RemoteFileIndex *index, struct Cache *cache,
    GError **error) {
  gchar **envp_hooked =
    g_environ_setenv(g_strdupv(envp), "LD_PRELOADa", hookfs, TRUE);
  int ret = Subprocess_init(
    (struct Subprocess *) p, argv, envp_hooked, selfpath, NULL, NULL, error);
  g_strfreev(envp_hooked);
  should (ret == 0) otherwise return ret;

  p->index = index;
  p->cache = cache;
  p->outputs = g_hash_table_new_full(
    g_str_hash, g_str_equal, NULL, HookedSubprocessOutput_free);

  return ret;
}


void HookedSubprocess__destruct (void) {
  g_thread_pool_free(HookedSubprocess__threadpool, FALSE, TRUE);
  HookedSubprocess__threadpool = NULL;
}


int HookedSubprocess__construct (GError **error) {
  HookedSubprocess__threadpool =
    g_thread_pool_new((GFunc) HookedSubprocess_join, NULL, -1, FALSE, error);
  return HookedSubprocess__threadpool == NULL;
}
