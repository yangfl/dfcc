#ifndef DFCC_SPAWN_SUBPROCESS_H
#define DFCC_SPAWN_SUBPROCESS_H
/**
 * @defgroup Spawn
 * @brief Spawn a new process
 */

#include <stdbool.h>
#include <threads.h>

#include <glib.h>

BEGIN_C_DECLS


//! @ingroup Spawn
extern GQuark DFCC_SPAWN_ERROR;


struct Process;
//! @memberof Process
typedef void (*ProcessExitCallback) (struct Process *);


/**
 * @ingroup Spawn
 * @brief Contains the information of a spawned process.
 */
struct Process {
  //! PID of the process.
  GPid pid;

  //! File descriptor for stdin of the process.
  gint stdin;
  //! File descriptor for stdout of the process.
  gint stdout;
  //! File descriptor for stderr of the process.
  gint stderr;

  //! Whether the process has stopped.
  bool stopped;
  //! Mutex for events.
  mtx_t mtx;
  //! Exit error if any.
  GError *error;
  //! Callback when finish.
  ProcessExitCallback onexit;
  //! User data.
  void *userdata;
};


/**
 * @memberof Process
 * @brief Frees associated resources of a Process.
 *
 * If the child program has not stopped, it will be killed and a warning will be
 * displayed.
 *
 * @param p a Process
 */
void Process_destroy (struct Process *p);
/**
 * @memberof Process
 * @brief Initializes a Process and executes a child program with given
 *        `argv` and `envp`.
 *
 * The child program is specified by the only argument that must be provided,
 * `argv`. `argv` should be a NULL-terminated array of strings, to be passed as
 * the argument vector for the child.
 *
 * The first string in `argv` is of course the name of the program to execute.
 * If the name of the program is not a full path, the `PATH` environment
 * variable is used to search for the executable, with `selfpath` avoided.
 *
 * `DFCC_LOOP_DETECTION_ENV` is automatically added into `envp` for loop
 * detection of `dfcc` itself.
 *
 * If `p` is NULL, the child is executed synchronously, otherwise
 * asynchronously.
 *
 * @param p a Process [optional]
 * @param argv child's argument vector [array zero-terminated=1]
 * @param envp child's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param selfpath path to be avoided when searching `argv[0]` in `PATH`
 *                 [optional]
 * @param onexit Callback when process exits [optional]
 * @param userdata User data [optional]
 * @param[out] error a return location for a GError [optional]
 * @return the exit status of the spawned process if synchronously, otherwise 0
 *         if success, otherwize nonzero
 */
int Process_init (
  struct Process *p, gchar **argv, gchar **envp, const char *selfpath,
  ProcessExitCallback onexit, void *userdata, GError **error);


END_C_DECLS

#endif /* DFCC_SPAWN_SUBPROCESS_H */
