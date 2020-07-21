#ifndef DFCC_SPAWN_HOOKED_SUBPROCESS_H
#define DFCC_SPAWN_HOOKED_SUBPROCESS_H

#include <glib.h>

#include "file/cache.h"
#include "file/hash.h"
#include "file/remoteindex.h"
#include "spawn/hookedprocessgroup.h"
#include "spawn/process.h"


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
  struct HookedProcessOutput *output =
    g_malloc(sizeof(struct HookedProcessOutput));
  should (HookedProcessOutput_init(
      output, path, mode, error) == 0) otherwise {
    g_free(output);
    return NULL;
  }
  return output;
}


struct HookedProcess;
//! @memberof Process
typedef void (*HookedProcessExitCallback) (struct HookedProcess *);


/**
 * @ingroup Spawn
 * @extends Process
 * @brief Contains the information of a process with hookfs preloaded.
 */
struct HookedProcess {
  struct Process;
  struct HookedProcessGroup *group;
  GHashTable *outputs;
  HookedProcessExitCallback onexit_hooked;
  const char *missing_path;
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
 * @param onexit Callback when process exits [optional]
 * @param userdata User data [optional]
 * @param group a HookedProcessGroup
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
*/
int HookedProcess_init (
  struct HookedProcess *p, gchar **argv, gchar **envp,
  HookedProcessExitCallback onexit, void *userdata,
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
 * @param onexit Callback when process exits [optional]
 * @param userdata User data [optional]
 * @param group a HookedProcessGroup
 * @param[out] error a return location for a GError [optional]
 * @return a HookedProcess [transfer-full]
*/
struct HookedProcess *HookedProcess_new (
  gchar **argv, gchar **envp, HookedProcessExitCallback onexit, void *userdata,
  struct HookedProcessGroup *group, GError **error);


#endif /* DFCC_SPAWN_HOOKED_SUBPROCESS_H */
