#include <stdbool.h>
#include <string.h>

#include <glib.h>

#include <macro.h>

#include "../spawn/subprocess.h"
#include "session.h"
#include "job.h"


extern inline void Job_destroy (struct Job *job);
extern inline struct Job *Job_new (
  SessionID sid,
  char **argv, char **envp, const char *working_directory,
  const char *hookfs, const char *selfpath, GError **error);


void Job_free (void *job) {
  Job_destroy(job);
  g_free(job);
}


int Job_init (struct Job *job, SessionID sid,
    char **argv, char **envp, const char *working_directory,
    const char *hookfs, const char *selfpath, GError **error) {
  int ret = Subprocess_init(&job->p, argv, envp, selfpath, NULL, NULL, error);
  should (ret == 0) otherwise {
    printf("%d\n",ret);
    return ret;
  }

  job->jid = job->p.pid;
  job->sid = sid;

  return 0;
}


extern inline bool JobTable_is_full (struct JobTable *jobtable);


static gboolean JobTable_clean_foreach_remove_cb (
    gpointer key, gpointer value, gpointer user_data) {
  GHashTable *session_table = (GHashTable *) user_data;
  struct Job *job = (struct Job *) value;
  return !g_hash_table_contains(session_table, &job->sid);
}


unsigned int JobTable_clean (
    struct JobTable *jobtable, struct SessionTable *session_table) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&session_table->rwlock);
  g_rw_lock_writer_lock(&jobtable->rwlock);
  unsigned int n_removed = g_hash_table_foreach_remove(
    jobtable->table, JobTable_clean_foreach_remove_cb, session_table->table);
  g_rw_lock_writer_unlock(&jobtable->rwlock);
  g_rw_lock_reader_locker_free(locker);
  return n_removed;
}


struct Job *JobTable_lookup (
    struct JobTable *jobtable, SessionID sid, JobID jid) {
  struct Job *job = g_hash_table_lookup(jobtable->table, &jid);
  return_if_fail(job != NULL && job->sid == sid) NULL;
  return job;
}


/**
 * @related JobTable
 * @brief Callback when a compiler process ends.
 *
 * @param spawn a Subprocess
 * @param userdata pointer to a JobTable
 */
static void JobTable_onjobfinish (struct Subprocess *spawn, void *userdata) {
  struct JobTable *jobtable = (struct JobTable *) userdata;

  mtx_lock(&jobtable->counter_mutex);
  jobtable->n_running--;
  mtx_unlock(&jobtable->counter_mutex);
}


void JobTable_insert (struct JobTable *jobtable, struct Job *job, bool pending) {
  mtx_lock(&jobtable->counter_mutex);
  if (pending) {
    jobtable->n_pending--;
  }
  if (job != NULL) {
    jobtable->n_running++;
  }
  mtx_unlock(&jobtable->counter_mutex);

  if (job != NULL) {
    job->p.onexit = JobTable_onjobfinish;
    job->p.onexit_userdata = jobtable;

    g_rw_lock_writer_lock(&jobtable->rwlock);
    g_hash_table_insert(jobtable->table, &job->jid, job);
    g_rw_lock_writer_unlock(&jobtable->rwlock);
  }
}


bool JobTable_try_reserve (struct JobTable *jobtable) {
  if (JobTable_is_full(jobtable)) {
    return false;
  }

  bool ret;

  mtx_lock(&jobtable->counter_mutex);
  if (JobTable_is_full(jobtable)) {
    ret = false;
  } else {
    jobtable->n_pending++;
    ret = true;
  }
  mtx_unlock(&jobtable->counter_mutex);
  return ret;
}


struct Job *JobTable_new (struct JobTable *jobtable, SessionID sid,
    char **argv, char **envp, const char *working_directory,
    const char *hookfs, const char *selfpath, GError **error) {
  struct Job *job = Job_new(
    sid, argv, envp, working_directory, hookfs, selfpath, error);
  JobTable_insert(jobtable, job, true);
  return job;
}


void JobTable_destroy (struct JobTable *jobtable) {
  g_hash_table_destroy(jobtable->table);
  g_rw_lock_clear(&jobtable->rwlock);
  mtx_destroy(&jobtable->counter_mutex);
}


int JobTable_init (struct JobTable *jobtable, unsigned int max_njob) {
  jobtable->table = g_hash_table_new_full(
    g_int_hash, g_int_equal, NULL, Job_free);
  g_rw_lock_init(&jobtable->rwlock);
  mtx_init(&jobtable->counter_mutex, mtx_plain);
  jobtable->n_pending = 0;
  jobtable->n_running = 0;
  jobtable->max_njob = max_njob;
  return 0;
}
