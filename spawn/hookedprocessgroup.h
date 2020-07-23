#ifndef DFCC_SPAWN_HOOKED_SUBPROCESS_GROUP_H
#define DFCC_SPAWN_HOOKED_SUBPROCESS_GROUP_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>

#include <glib.h>

#include "file/remoteindex.h"
#include "spawn/_hookedprocessgroupid.h"
#include "spawn/hookedprocess.h"
#include "spawn/hookfsserver.h"

BEGIN_C_DECLS


/**
 * @ingroup Spawn
 * @brief Contains the information of HookedProcessGroup.
 */
struct HookedProcessGroupManager {
  struct HookFsServer ANON_MEMBER;
  /// Hash table mapping HookedProcessGroupID to HookedProcessGroup.
  GHashTable *table;
  /// Lock for `table`.
  GRWLock rwlock;

  /// Number of currently available job slots.
  atomic_int n_available;
  /// Preload libSegFault.so.
  bool debug;
  /// Path to the executable file of `dfcc`.
  const char *selfpath;
  /// Path to the preload library `hookfs`.
  const char *hookfs;

  /// Source file cache.
  struct Cache cache;
  cnd_t cond;
  mtx_t cond_mtx;

  void (*onmissing) (struct HookedProcessGroupManager *, void *, char *);
  void *onmissing_userdata;
};


/**
 * @memberof HookedProcessGroupManager
 * @brief Looks up a HookedProcessGroup in a HookedProcessGroupManager.
 *
 * @param manager a HookedProcessGroupManager
 * @param hgid a HookedProcessGroupID
 * @return HookedProcessGroup [nullable]
 */
struct HookedProcessGroup *HookedProcessGroupManager_lookup (
  struct HookedProcessGroupManager *manager, HookedProcessGroupID hgid);
/**
 * @memberof HookedProcessGroupManager
 * @brief Frees associated resources of a HookedProcessGroupManager.
 *
 * @param manager a HookedProcessGroupManager
 */
void HookedProcessGroupManager_destroy (
  struct HookedProcessGroupManager *manager);
/**
 * @memberof HookedProcessGroupManager
 * @brief Initializes a HookedProcessGroupManager.
 *
 * @param manager a HookedProcessGroupManager
 * @param max_njob maximum number of simultaneously jobs
 * @return 0 if success, otherwize nonzero
 */
int HookedProcessGroupManager_init (
  struct HookedProcessGroupManager *manager, unsigned int jobs,
  const char *selfpath, const char *hookfs, const char *socket_path,
  const char *cache_dir, bool no_verify_cache, GError **error);


/**
 * @ingroup Spawn
 * @brief Contains the information of all HookedProcess.
 */
struct HookedProcessGroup {
  /// Hash table mapping GPid to Job.
  GHashTable *table;
  /// Lock for `table`.
  GRWLock rwlock;
  /// Group ID.
  HookedProcessGroupID hgid;
  /// Group ID string.
  char s_hgid[2 * sizeof(HookedProcessGroupID) + 1];
  /// Meta table.
  struct HookedProcessGroupManager *manager;
  /// Files of the remote client.
  struct RemoteFileIndex file_index;
  /// Virtual destructor.
  void (*destructor) (void *);
  /// User data.
  void *userdata;
};


/**
 * @memberof HookedProcessGroup
 * @brief Looks up a job in a HookedProcessGroup.
 *
 * @param group a HookedProcessGroup
 * @param pid a GPid
 * @return a HookedProcess [nullable]
 */
struct HookedProcess *HookedProcessGroup_lookup (
  struct HookedProcessGroup *group, GPid pid);
/**
 * @related HookedProcessGroup
 * @brief Callback when a compiler process ends.
 *
 * @param spawn a Process
 * @param userdata pointer to a HookedProcessGroup
 */
void HookedProcessGroup_onexit (struct Process *p_);
/**
 * @memberof HookedProcessGroup
 * @brief Try to reserve a job slot for a client, by increasing
 *        HookedProcessGroup.n_pending by 1.
 *
 * @param group a HookedProcessGroup
 * @return `true` if a slot is reserved
 */
bool HookedProcessGroup_reserve (struct HookedProcessGroup *group);
/**
 * @memberof HookedProcessGroup
 * @brief Creates a new Job, starts the compiler, and inserts the Job into
 *        `group`.
 *
 * @param group a HookedProcessGroup
 * @param argv compiler's argument vector [array zero-terminated=1]
 * @param envp compiler's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param onexit Callback when process exits [optional]
 * @param userdata User data [optional]
 * @param[out] error a return location for a GError [optional]
 * @return Job [transfer-none]
 */
struct HookedProcess *HookedProcessGroup_new_job (
  struct HookedProcessGroup *group, gchar **argv, gchar **envp,
  HookedProcessExitCallback onexit, void *userdata, GError **error);
/**
 * @memberof HookedProcessGroup
 * @brief Frees associated resources of a HookedProcessGroup.
 *
 * @param group a HookedProcessGroup
 */
void HookedProcessGroup_destroy (struct HookedProcessGroup *group);
/**
 * @memberof HookedProcessGroup
 * @brief Frees associated resources of a HookedProcessGroup.
 *
 * @param group a HookedProcessGroup
 */
void HookedProcessGroup_virtual_destroy (void *group);
/**
 * @memberof HookedProcessGroup
 * @brief Frees a HookedProcessGroup and its associated resources.
 *
 * @param group a HookedProcessGroup
 */
void HookedProcessGroup_free (void *group);
/**
 * @memberof HookedProcessGroup
 * @brief Initializes a HookedProcessGroup.
 *
 * @param group a HookedProcessGroup
 * @param max_njob maximum number of simultaneously jobs
 * @return 0 if success, otherwize nonzero
 */
int HookedProcessGroup_init (
  struct HookedProcessGroup *group, HookedProcessGroupID hgid,
  struct HookedProcessGroupManager *manager);


END_C_DECLS

#endif /* DFCC_SPAWN_HOOKED_SUBPROCESS_GROUP_H */
