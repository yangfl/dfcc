#ifndef DFCC_SPAWN_SUBPROCESS_H
#define DFCC_SPAWN_SUBPROCESS_H
/**
 * @defgroup Spawn
 * @brief Spawn a new process
 */

#include <stdbool.h>

#include <glib.h>


//! @ingroup Spawn
extern GQuark DFCC_SPAWN_ERROR;


struct Subprocess;
//! @memberof Subprocess
typedef void (*SubprocessExitCallback) (struct Subprocess *, void *);


/**
 * @ingroup Spawn
 * @brief Contains the information of a spawned process.
 */
struct Subprocess {
  GPid pid;

  //! File descriptor for stdin of the process
  gint stdin;
  //! File descriptor for stdout of the process
  gint stdout;
  //! File descriptor for stderr of the process
  gint stderr;

  //! Whether the process has stopped
  bool stopped;
  //! Exit error if any
  GError *error;
  //! Callback when finish
  SubprocessExitCallback onexit;
  //! Additional userdata
  void *onexit_userdata;
};

/**
 * @memberof Subprocess
 * @brief Frees associated resources of a Subprocess.
 *
 * The function does not check whether the child program has stopped.
 *
 * @param p a Subprocess
 */
void Subprocess_destroy (struct Subprocess *p);
/**
 * @memberof Subprocess
 * @brief Initializes a Subprocess and executes a child program with given
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
 * @param p a Subprocess [optional]
 * @param argv child's argument vector [array zero-terminated=1]
 * @param envp child's environment, or NULL to inherit parent's
 *             [array zero-terminated=1][optional]
 * @param selfpath path to be avoided when searching `argv[0]` in `PATH`
 *                 [optional]
 * @param[out] error a return location for a GError [optional]
 * @return the exit status of the spawned process if synchronously, otherwise 0
 *         if success, otherwize nonzero
 */
int Subprocess_init (
  struct Subprocess *p, gchar **argv, gchar **envp, const char *selfpath,
  SubprocessExitCallback onexit, void *onexit_userdata, GError **error);


#endif /* DFCC_SPAWN_SUBPROCESS_H */
