#ifndef DFCC_SERVER_JOB_H
#define DFCC_SERVER_JOB_H

#include <stdbool.h>
#include <threads.h>

#include <glib.h>

#include "../spawn/subprocess.h"
#include "session.h"


/**
 * @memberof Job
 * @brief Job ID
 */
typedef GPid JobID;


/**
 * @ingroup Server
 * @brief Contains the information of a compiling job.
 */
struct Job {
  /// Unique ID for this job.
  JobID jid;
  /// SID of the associated session.
  SessionID sid;
  /// Associated compiler subprocess.
  struct Subprocess p;
};


/**
 * @memberof Job
 * @brief Frees associated resources of a Job.
 *
 * @param job a Job
 */
inline void Job_destroy (struct Job *job) {
  Subprocess_destroy(&job->p);
}

/**
 * @memberof Job
 * @brief Frees a Job and associated resources.
 *
 * @param job a Job
 */
void Job_free (void *job);
/**
 * @memberof Job
 * @brief Initializes a Job and starts the compiler.
 *
 * @param job a Job
 * @param sid Session ID
 * @param argv compiler's argument vector [array zero-terminated=1]
 * @param envp compiler's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param working_directory compiler's working directory, remotely
 * @param hookfs path to the preload library `hookfs`
 * @param selfpath path to the executable of `dfcc`
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
*/
int Job_init (
  struct Job *job, SessionID sid,
  char **argv, char **envp, const char *working_directory,
  const char *hookfs, const char *selfpath, GError **error);
/**
 * @memberof Job
 * @brief Create a new Job and starts the compiler.
 *
 * @param job a Job
 * @param sid Session ID
 * @param argv compiler's argument vector [array zero-terminated=1]
 * @param envp compiler's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param working_directory compiler's working directory, remotely
 * @param hookfs path to the preload library `hookfs`
 * @param selfpath path to the executable of `dfcc`
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
inline struct Job *Job_new (
    SessionID sid,
    char **argv, char **envp, const char *working_directory,
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


/**
 * @ingroup Server
 * @brief Contains the information of all jobs.
 *
 * @sa Job
 */
struct JobTable {
  /// Hash table mapping JobID to Job.
  GHashTable *table;
  /// Lock for `table`.
  GRWLock rwlock;
  /// Lock for `n_pending` and `n_running`.
  mtx_t counter_mutex;
  /**
   * @brief Number of jobs to be submitted by clients.
   * @sa JobTable_try_reserve
   */
  unsigned int n_pending;
  /**
   * @brief Number of currently running jobs.
   *
   * Not necessarily equal to the number of elements in JobTable.table.
   */
  unsigned int n_running;
  /// Maximum number of possible jobs the server can handle simultaneously.
  unsigned int max_njob;
};


/**
 * @memberof JobTable
 * @brief Tests if the server can accept more jobs.
 *
 * @param jobtable a JobTable
 * @return `true` if the server can accept jobs
 */
inline bool JobTable_is_full (struct JobTable *jobtable) {
  return jobtable->n_pending + jobtable->n_running >= jobtable->max_njob;
}

/**
 * @memberof JobTable
 * @brief Cleans jobs without a session.
 *
 * @param jobtable a JobTable
 * @param session_table a SessionTable
 * @return the number of jobs removed
 */
unsigned int JobTable_clean (
  struct JobTable *jobtable, struct SessionTable *session_table);
/**
 * @memberof JobTable
 * @brief Looks up a job in a JobTable.
 *
 * @param jobtable a JobTable
 * @return `true` if a slot is reserved
 */
struct Job *JobTable_lookup (
  struct JobTable *jobtable, SessionID sid, JobID jid);
/**
 * @memberof JobTable
 * @brief Inserts a Job into a JobTable.
 *
 * @param jobtable a JobTable
 * @param job a Job [nullable]
 * @param pending whether the job slot has been reserved
 */
void JobTable_insert (struct JobTable *jobtable, struct Job *job, bool pending);
/**
 * @memberof JobTable
 * @brief Try to reserve a job slot for a client, by increasing
 *        JobTable.n_pending by 1.
 *
 * @param jobtable a JobTable
 * @return `true` if a slot is reserved
 */
bool JobTable_try_reserve (struct JobTable *jobtable);
/**
 * @memberof JobTable
 * @brief Creates a new Job, starts the compiler, and inserts the Job into
 *        `jobtable`.
 *
 * @param jobtable a JobTable
 * @param sid Session ID
 * @param argv compiler's argument vector [array zero-terminated=1]
 * @param envp compiler's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param working_directory compiler's working directory, remotely
 * @param hookfs path to the preload library `hookfs`
 * @param selfpath path to the executable of `dfcc`
 * @param[out] error a return location for a GError [optional]
 * @return Job [transfer-none]
 */
struct Job *JobTable_new (struct JobTable *jobtable, SessionID sid,
  char **argv, char **envp, const char *working_directory,
  const char *hookfs, const char *selfpath, GError **error);
/**
 * @memberof JobTable
 * @brief Frees associated resources of a JobTable.
 *
 * @param jobtable a JobTable
 */
void JobTable_destroy (struct JobTable *jobtable);
/**
 * @memberof JobTable
 * @brief Initializes a JobTable.
 *
 * @param jobtable a JobTable
 * @param max_njob maximum number of simultaneously jobs
 * @return 0 if success, otherwize nonzero
 */
int JobTable_init (struct JobTable *jobtable, unsigned int max_njob);


#endif /* DFCC_SERVER_JOB_H */
