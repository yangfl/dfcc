#ifndef DFCC_SERVER_JOB_H
#define DFCC_SERVER_JOB_H

#include <stdbool.h>

#include <glib.h>

#include "../spawn/subprocess.h"
#include "session.h"


typedef GPid JobID;


struct Job {
  // Unique ID for this job.
  JobID jid;

  // SID of the associated session.
  SessionID sid;

  struct Subprocess p;
};


inline void Job_destroy (struct Job *job) {
  Subprocess_destroy(&job->p);
}

void Job_free (struct Job *job);
int Job_init (
    struct Job *job, SessionID sid,
    gchar **argv, gchar **envp, const gchar *working_directory,
    const char *hookfs, const char *selfpath, GError **error);

inline struct Job *Job_new (
    SessionID sid,
    gchar **argv, gchar **envp, const gchar *working_directory,
    const char *hookfs, const char *selfpath, GError **error) {
  struct Job *job = g_malloc(sizeof(struct Job));
  should (Job_init(
      job, sid, argv, envp, working_directory,
      hookfs, selfpath, error) == 0) otherwise {
    g_free(job);
    return NULL;
  }
  return job;
}


struct JobTable {
  GHashTable *table;
  GRWLock rwlock;
  GMutex counter_mutex;
  unsigned int npending;
  unsigned int nrunning;
  unsigned int max_njob;
};


inline bool JobTable_full (struct JobTable *jobtable) {
  return jobtable->npending + jobtable->nrunning >= jobtable->max_njob;
}

void JobTable_insert (struct JobTable *jobtable, struct Job *job);
bool JobTable_try_reserve (struct JobTable *jobtable);
void JobTable_destroy (struct JobTable *jobtable);
int JobTable_init (struct JobTable *jobtable, unsigned int max_njob);

#endif /* DFCC_SERVER_JOB_H */
