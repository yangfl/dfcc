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
    gchar **argv, gchar **envp, const gchar *working_directory,
    const char *hookfs, const char *selfpath, GError **error);


void Job_free (struct Job *job) {
  Job_destroy(job);
  g_free(job);
}


int Job_init (struct Job *job, SessionID sid,
    gchar **argv, gchar **envp, const gchar *working_directory,
    const char *hookfs, const char *selfpath, GError **error) {
  int ret = Subprocess_init(&job->p, argv, envp, selfpath, error);
  should (ret == 0) otherwise {
    printf("%d\n",ret);
    return ret;
  }

  job->jid = job->p.pid;
  job->sid = sid;

  return 0;
}


extern inline bool JobTable_full (struct JobTable *jobtable);


/**
 * @related JobTable
 * @brief Callback when a compiler process ends.
 *
 * @param spawn a Subprocess
 * @param userdata pointer to a JobTable
 */
static void JobTable_onjobfinish (struct Subprocess *spawn, void *userdata) {
  struct JobTable *jobtable = (struct JobTable *) userdata;

  g_mutex_lock(&jobtable->counter_mutex);
  jobtable->nrunning--;
  g_mutex_unlock(&jobtable->counter_mutex);
}


void JobTable_insert (struct JobTable *jobtable, struct Job *job) {
  g_mutex_lock(&jobtable->counter_mutex);
  jobtable->npending--;
  if (job != NULL) {
    jobtable->nrunning++;
  }
  g_mutex_unlock(&jobtable->counter_mutex);

  if (job != NULL) {
    job->p.onfinish = JobTable_onjobfinish;
    job->p.userdata = jobtable;

    g_rw_lock_writer_lock(&jobtable->rwlock);
    g_hash_table_insert(jobtable->table, &job->jid, job);
    g_rw_lock_writer_unlock(&jobtable->rwlock);
  }
}


bool JobTable_try_reserve (struct JobTable *jobtable) {
  if (JobTable_full(jobtable)) {
    return false;
  }

  bool ret;

  g_mutex_lock(&jobtable->counter_mutex);
  if (JobTable_full(jobtable)) {
    ret = false;
  } else {
    jobtable->npending++;
    ret = true;
  }
  g_mutex_unlock(&jobtable->counter_mutex);
  return ret;
}


void JobTable_destroy (struct JobTable *jobtable) {
  g_hash_table_destroy(jobtable->table);
  g_rw_lock_clear(&jobtable->rwlock);
  g_mutex_clear(&jobtable->counter_mutex);
}


int JobTable_init (struct JobTable *jobtable, unsigned int max_njob) {
  jobtable->table = g_hash_table_new_full(
    g_int_hash, g_int_equal, NULL, (void (*)(void *)) Job_free);
  g_rw_lock_init(&jobtable->rwlock);
  g_mutex_init(&jobtable->counter_mutex);
  jobtable->npending = 0;
  jobtable->nrunning = 0;
  jobtable->max_njob = max_njob;
  return 0;
}
