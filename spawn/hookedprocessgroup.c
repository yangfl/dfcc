#include <stdbool.h>

#include <libsoup/soup.h>
#include <glib.h>

#include "common/macro.h"
#include "common/atomiccount.h"
#include "file/cache.h"
#include "hookedprocessgroup.h"


struct HookedProcessGroup *HookedProcessGroupManager_lookup (
    struct HookedProcessGroupManager *manager, HookedProcessGroupID hgid) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&manager->rwlock);
  struct HookedProcessGroup *group = g_hash_table_lookup(
    manager->table, &hgid);
  g_rw_lock_reader_locker_free(locker);
  return group;
}


/**
 * @memberof HookedProcessGroupManager
 * @brief Looks up a job with `hgid` and `pid`.
 *
 * @param manager_ a HookedProcessGroupManager
 * @param gpid a HookedProcessGroupID
 * @param pid a GPid
 * @return a HookedProcess [nullable]
 */
static struct HookedProcess *HookedProcessGroupManager_resolve (
    void *manager_, HookedProcessGroupID hgid, GPid pid) {
  struct HookedProcessGroupManager *manager =
    (struct HookedProcessGroupManager *) manager_;
  struct HookedProcessGroup *group = HookedProcessGroupManager_lookup(
    manager, hgid);
  return_if_fail(group != NULL) NULL;
  return HookedProcessGroup_lookup(group, pid);
}


void HookedProcessGroupManager_destroy (
    struct HookedProcessGroupManager *manager) {
  HookFsServer_destroy((struct HookFsServer *) manager);
  Cache_destroy(&manager->cache);
  g_rw_lock_writer_lock(&manager->rwlock);
  g_hash_table_destroy(manager->table);
  g_rw_lock_writer_unlock(&manager->rwlock);
  g_rw_lock_clear(&manager->rwlock);
}


int HookedProcessGroupManager_init (
    struct HookedProcessGroupManager *manager, unsigned int jobs,
    const char *selfpath, const char *hookfs, const char *socket_path,
    const char *cache_dir, bool no_verify_cache, GError **error) {
  return_if_fail(HookFsServer_init(
    (struct HookFsServer *) manager, socket_path,
    NULL, HookedProcessGroupManager_resolve, error
  ) == 0) 1;
  should (Cache_init(
      &manager->cache, cache_dir, no_verify_cache) == 0) otherwise {
    HookFsServer_destroy((struct HookFsServer *) manager);
    return 1;
  }

  manager->table = g_hash_table_new_full(
    g_int_hash, g_int_equal, NULL, HookedProcessGroup_free);
  g_rw_lock_init(&manager->rwlock);

  manager->n_available = jobs;
  manager->debug = 1; // temp
  manager->selfpath = selfpath;
  manager->hookfs = hookfs;
  return 0;
}


struct HookedProcess *HookedProcessGroup_lookup (
    struct HookedProcessGroup *group, GPid pid) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&group->rwlock);
  struct HookedProcess *p = g_hash_table_lookup(group->table, &pid);
  g_rw_lock_reader_locker_free(locker);
  return_if_fail(p != NULL) NULL;
  return p;
}


void HookedProcessGroup_onexit (struct Process *p_) {
  struct HookedProcess *p = (struct HookedProcess *) p_;
  p->group->manager->n_available++;
  if (p->onexit_hooked != NULL) {
    p->onexit_hooked(p);
  }
}


/**
 * @memberof HookedProcessGroup
 * @brief Inserts a Job into a HookedProcessGroup.
 *
 * @param group a HookedProcessGroup
 * @param job a Job [nullable]
 * @param pending whether the job slot has been reserved
 */
static void HookedProcessGroup_insert (
    struct HookedProcessGroup *group, struct HookedProcess *p,
    bool pending) {
  should (p != NULL) otherwise {
    if (pending) {
      group->manager->n_available++;
    }
    return;
  }

  if (!pending) {
    group->manager->n_available--;
  }

  g_rw_lock_writer_lock(&group->rwlock);
  g_hash_table_insert(group->table, &p->pid, p);
  g_rw_lock_writer_unlock(&group->rwlock);
}


bool HookedProcessGroup_reserve (struct HookedProcessGroup *group) {
  return count_dec(&group->manager->n_available);
}


struct HookedProcess *HookedProcessGroup_new_job (
    struct HookedProcessGroup *group, gchar **argv, gchar **envp,
    HookedProcessExitCallback onexit, void *userdata, GError **error) {
  should (HookedProcessGroup_reserve(group)) otherwise {
    g_set_error_literal(
      error, DFCC_SPAWN_ERROR, SOUP_STATUS_SERVICE_UNAVAILABLE, "Server full");
    return NULL;
  }
  struct HookedProcess *p = HookedProcess_new(
    argv, envp, onexit, userdata, group, error);
  return_if_fail(p != NULL) NULL;
  HookedProcessGroup_insert(group, p, true);
  return p;
}


void HookedProcessGroup_destroy (struct HookedProcessGroup *group) {
  g_rw_lock_writer_lock(&group->rwlock);
  g_hash_table_destroy(group->table);
  g_rw_lock_writer_unlock(&group->rwlock);
  g_rw_lock_clear(&group->rwlock);
  RemoteFileIndex_destroy(&group->file_index);
}


void HookedProcessGroup_virtual_destroy (void *group) {
  ((struct HookedProcessGroup *) group)->destructor(group);
}


void HookedProcessGroup_free (void *group) {
  HookedProcessGroup_virtual_destroy(group);
  g_free(group);
}


int HookedProcessGroup_init (
    struct HookedProcessGroup *group, HookedProcessGroupID hgid,
    struct HookedProcessGroupManager *manager) {
  return_if_fail(RemoteFileIndex_init(&group->file_index) == 0) 1;

  group->table = g_hash_table_new_full(
    g_int_hash, g_int_equal, NULL, HookedProcess_free);
  g_rw_lock_init(&group->rwlock);

  group->hgid = hgid;
  snprintf(group->s_hgid, sizeof(group->s_hgid), "%x", group->hgid);
  group->manager = manager;
  group->destructor = (void (*) (void *)) HookedProcessGroup_destroy;
  return 0;
}
