#ifndef DFCC_SPAWN_HOOKED_SUBPROCESS_H
#define DFCC_SPAWN_HOOKED_SUBPROCESS_H

#include <glib.h>

#include "../file/cache.h"
#include "../file/hash.h"
#include "../file/remoteindex.h"
#include "hookfsserver.h"
#include "subprocess.h"


struct HookedSubprocessOutput {
  char *path;
  char *tmp_path;
  int mode;
  int fd;
};


void HookedSubprocessOutput_destroy (struct HookedSubprocessOutput *output);
void HookedSubprocessOutput_free (void *output);
int HookedSubprocessOutput_init (
    struct HookedSubprocessOutput *output, gchar *path,
    int mode, GError **error);

inline struct HookedSubprocessOutput *HookedSubprocessOutput_new (
    gchar *path, int mode, GError **error) {
  struct HookedSubprocessOutput *output =
    g_malloc(sizeof(struct HookedSubprocessOutput));
  should (HookedSubprocessOutput_init(
      output, path, mode, error) == 0) otherwise {
    g_free(output);
    return NULL;
  }
  return output;
}


struct HookedSubprocess;
typedef void (*HookedSubprocessNextFunc) (struct HookedSubprocess *);

enum HookedSubprocessPendingType {
  HOOKED_PENDING_NONE = 0,
  HOOKED_PENDING_PATH_HASH,
  HOOKED_PENDING_HASH_FILE
};

struct HookedSubprocess {
  struct Subprocess;

  GString *stdout_buf;

  enum HookedSubprocessPendingType pending_type;
  union {
    char *pending_path;
    struct FileHash pending_hash;
  };

  struct RemoteFileIndex *index;
  struct Cache *cache;
  GHashTable *outputs;

  HookedSubprocessNextFunc onmissing;
  void *onmissing_userdata;
};


gboolean HookedSubprocess_run (struct HookedSubprocess *p, GError **error);
void HookedSubprocess_destroy (struct HookedSubprocess *p);
int HookedSubprocess_init (
    struct HookedSubprocess *p, gchar **argv, gchar **envp,
    const char *hookfs, const char *selfpath,
    struct RemoteFileIndex *index, struct Cache *cache,
    GError **error);


#endif /* DFCC_SPAWN_HOOKED_SUBPROCESS_H */
