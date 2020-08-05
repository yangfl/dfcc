#ifndef DFCC_SPAWN_HOOKED_SUBPROCESS_H
#define DFCC_SPAWN_HOOKED_SUBPROCESS_H

#include <glib.h>

#include "file/cache.h"
#include "file/hash.h"
#include "file/remoteindex.h"
#include "hookedprocessgroup.h"
#include "process.h"

BEGIN_C_DECLS


/**
 * @ingroup Spawn
 *
 * @sa HookedProcess
 */
struct HookedProcessOutput {
  char *path;
  char *tmp_path;
  int mode;
  int fd;
};


/**
 * @memberof HookedProcessOutput
 * @brief Frees associated resources of a HookedProcessOutput.
 *
 * @param output a HookedProcessOutput
 */
void HookedProcessOutput_destroy (struct HookedProcessOutput *output);
/**
 * @memberof HookedProcessOutput
 * @brief Frees a HookedProcessOutput and associated resources.
 *
 * @param output a HookedProcessOutput
 */
void HookedProcessOutput_free (void *output);
//! @memberof HookedProcessOutput
int HookedProcessOutput_init (
    struct HookedProcessOutput *output, gchar *path,
    int mode, GError **error);

//! @memberof HookedProcessOutput
inline struct HookedProcessOutput *HookedProcessOutput_new (
    gchar *path, int mode, GError **error) {
  struct HookedProcessOutput *output = g_new(struct HookedProcessOutput, 1);
  should (HookedProcessOutput_init(
      output, path, mode, error) == 0) otherwise {
    g_free(output);
    return NULL;
  }
  return output;
}


#define HOOKEDPROCESS_FILE_MISSING 1
#define HOOKEDPROCESS_OUTPUT 2


/**
 * @ingroup Spawn
 * @extends Process
 * @brief Contains the information of a process with hookfs preloaded.
 */
struct HookedProcess {
  struct Process ANON_MEMBER;
  struct HookedProcessGroup *group;
  ProcessOnchangeCallback onchange_hooked;

  GHashTable *outputs;
  const char *path;
  int mode;
};


/**
 * @memberof HookedProcess
 * @brief Frees associated resources of a HookedProcess.
 *
 * If the child program has not stopped, it will be killed and a warning will be
 * displayed.
 *
 * @param p a HookedProcess
 */
void HookedProcess_destroy (struct HookedProcess *p);
/**
 * @memberof HookedProcess
 * @brief Frees a HookedProcess and associated resources.
 *
 * @param p a HookedProcess
 */
void HookedProcess_free (void *p);
/**
 * @memberof HookedProcess
 * @brief Initializes a HookedProcess and executes a child program with given
 *        `argv` and `envp`.
 *
 * @param p a HookedProcess
 * @param argv compiler's argument vector [array zero-terminated=1]
 * @param envp compiler's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param selfpath path to the executable of `dfcc`
 * @param onchange Callback when process exits [optional]
 * @param userdata User data [optional]
 * @param group a HookedProcessGroup
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
*/
int HookedProcess_init (
  struct HookedProcess *p, gchar **argv, gchar **envp,
  ProcessOnchangeCallback onchange, void *userdata,
  struct HookedProcessGroup *group, GError **error);
/**
 * @memberof HookedProcess
 * @brief Create a new HookedProcess and executes a child program with given
 *        `argv` and `envp`.
 *
 * @param argv compiler's argument vector [array zero-terminated=1]
 * @param envp compiler's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param selfpath path to the executable of `dfcc`
 * @param onchange Callback when process exits [optional]
 * @param userdata User data [optional]
 * @param group a HookedProcessGroup
 * @param[out] error a return location for a GError [optional]
 * @return a HookedProcess [transfer-full]
*/
struct HookedProcess *HookedProcess_new (
  gchar **argv, gchar **envp, ProcessOnchangeCallback onchange, void *userdata,
  struct HookedProcessGroup *group, GError **error);


END_C_DECLS

#endif /* DFCC_SPAWN_HOOKED_SUBPROCESS_H */
