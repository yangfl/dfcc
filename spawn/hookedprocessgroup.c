#include <stdbool.h>

#include <libsoup/soup.h>
#include <glib.h>

#include "common/macro.h"
#include "common/atomiccount.h"
#include "file/cache.h"
#include "hookedprocessgroup.h"


struct HookedProcessGroup *HookedProcessController_lookup (
    struct HookedProcessController *controller, HookedProcessGroupID hgid) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&controller->rwlock);
  struct HookedProcessGroup *group = g_hash_table_lookup(
    controller->table, &hgid);
  g_rw_lock_reader_locker_free(locker);
  return group;
}


/**
 * @memberof HookedProcessController
 * @brief Looks up a job with `hgid` and `pid`.
 *
 * @param controller_ a HookedProcessController
 * @param gpid a HookedProcessGroupID
 * @param pid a GPid
 * @return a HookedProcess [nullable]
 */
static struct HookedProcess *HookedProcessController_resolve (
    void *controller_, HookedProcessGroupID hgid, GPid pid) {
  struct HookedProcessController *controller =
    (struct HookedProcessController *) controller_;
  struct HookedProcessGroup *group = HookedProcessController_lookup(controller, hgid);
  return_if_fail(group != NULL) NULL;
  return HookedProcessGroup_lookup(group, pid);
}


void HookedProcessController_destroy (
    struct HookedProcessController *controller) {
  HookFsServer_destroy((struct HookFsServer *) controller);
  Cache_destroy(&controller->cache);
  g_rw_lock_writer_lock(&controller->rwlock);
  g_hash_table_destroy(controller->table);
  g_rw_lock_writer_unlock(&controller->rwlock);
  g_rw_lock_clear(&controller->rwlock);
}


int HookedProcessController_init (
    struct HookedProcessController *controller, unsigned int jobs,
    const char *selfpath, const char *hookfs, const char *socket_path,
    const char *cache_dir, bool no_verify_cache, GError **error) {
  return_if_fail(HookFsServer_init(
    (struct HookFsServer *) controller, socket_path,
    NULL, HookedProcessController_resolve, error
  ) == 0) 1;
  should (Cache_init(
      &controller->cache, cache_dir, no_verify_cache) == 0) otherwise {
    HookFsServer_destroy((struct HookFsServer *) controller);
    return 1;
  }

  controller->table = g_hash_table_new_full(
    g_int_hash, g_int_equal, NULL, HookedProcessGroup_free);
  g_rw_lock_init(&controller->rwlock);

  controller->n_available = jobs;
  controller->debug = 1; // temp
  controller->selfpath = selfpath;
  controller->hookfs = hookfs;
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
  p->group->controller->n_available++;
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
      group->controller->n_available++;
    }
    return;
  }

  if (!pending) {
    group->controller->n_available--;
  }

  g_rw_lock_writer_lock(&group->rwlock);
  g_hash_table_insert(group->table, &p->pid, p);
  g_rw_lock_writer_unlock(&group->rwlock);
}


bool HookedProcessGroup_reserve (struct HookedProcessGroup *group) {
  return count_dec(&group->controller->n_available);
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
    struct HookedProcessController *controller) {
  return_if_fail(RemoteFileIndex_init(&group->file_index) == 0) 1;

  group->table = g_hash_table_new_full(
    g_int_hash, g_int_equal, NULL, HookedProcess_free);
  g_rw_lock_init(&group->rwlock);

  group->hgid = hgid;
  snprintf(group->s_hgid, sizeof(group->s_hgid), "%x", group->hgid);
  group->controller = controller;
  group->destructor = (void (*) (void *)) HookedProcessGroup_destroy;
  return 0;
}
