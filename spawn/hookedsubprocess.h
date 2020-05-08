#ifndef DFCC_SPAWN_HOOKED_SUBPROCESS_H
#define DFCC_SPAWN_HOOKED_SUBPROCESS_H

#include <glib.h>

#include "../file/cache.h"
#include "../file/hash.h"
#include "../file/remoteindex.h"
#include "hookfsserver.h"
#include "subprocess.h"


/**
 * @ingroup Spawn
 *
 * @sa HookedSubprocess
 */
struct HookedSubprocessOutput {
  char *path;
  char *tmp_path;
  int mode;
  int fd;
};


/**
 * @memberof HookedSubprocessOutput
 * @brief Frees associated resources of a HookedSubprocessOutput.
 *
 * @param output a HookedSubprocessOutput
 */
void HookedSubprocessOutput_destroy (struct HookedSubprocessOutput *output);
/**
 * @memberof HookedSubprocessOutput
 * @brief Frees a HookedSubprocessOutput and associated resources.
 *
 * @param output a HookedSubprocessOutput
 */
void HookedSubprocessOutput_free (void *output);
//! @memberof HookedSubprocessOutput
int HookedSubprocessOutput_init (
    struct HookedSubprocessOutput *output, gchar *path,
    int mode, GError **error);

//! @memberof HookedSubprocessOutput
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


/**
 * @ingroup Spawn
 * @extends Subprocess
 */
struct HookedSubprocess {
  struct Subprocess;

  struct RemoteFileIndex *index;
  struct Cache *cache;
  GHashTable *outputs;

  void (*onmissing) (struct HookedSubprocess *, void *, char *);
  void *onmissing_userdata;
};


//! @memberof HookedSubprocess
gboolean HookedSubprocess_run (struct HookedSubprocess *p, GError **error);
/**
 * @memberof HookedSubprocess
 * @brief Frees associated resources of a HookedSubprocess.
 *
 * The function does not check whether the child program has stopped.
 *
 * @param p a HookedSubprocess
 */
void HookedSubprocess_destroy (struct HookedSubprocess *p);
//! @memberof HookedSubprocess
int HookedSubprocess_init (
    struct HookedSubprocess *p, gchar **argv, gchar **envp,
    const char *hookfs, const char *selfpath,
    struct RemoteFileIndex *index, struct Cache *cache,
    GError **error);


#endif /* DFCC_SPAWN_HOOKED_SUBPROCESS_H */
