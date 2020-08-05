#include <stdbool.h>
#include <string.h>

#include <glib.h>

#include "common/macro.h"
#include "common/wrapper/threads.h"
#include "log.h"
#include "process.h"


GQuark DFCC_SPAWN_ERROR;


static void __attribute__ ((constructor)) DFCC_SPAWN_ERROR_init (void) {
  DFCC_SPAWN_ERROR = g_quark_from_static_string("dfcc-spawn-error-quark");
}


extern inline void Process_onchange (struct Process *p, int status);


static void Process_child_watch_cb (GPid pid, gint status, gpointer user_data) {
  struct Process *p = (struct Process *) user_data;

  CRITICAL_SECTIONS_START(&p->mtx, event);

  p->stopped = true;
  if (g_spawn_check_exit_status(status, &p->error)) {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_DEBUG,
          "Child %" G_PID_FORMAT " exited normally", pid);
  } else {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_INFO,
          "Child %" G_PID_FORMAT " exited abnormally: %s",
          pid, p->error->message);
  }
  Process_onchange(p, PROCESS_STATUS_EXIT);

  CRITICAL_SECTIONS_END(&p->mtx, event);
}


void Process_destroy (struct Process *p) {
  if (!p->stopped) {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
          "Destroy Process structure while child %" G_PID_FORMAT
          " has not stopped", p->pid);
  }
  g_spawn_close_pid(p->pid);

  GError *error = NULL;
  should (mtx_lock_e(&p->mtx, &error) == thrd_success) otherwise {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_ERROR,
          "%s: %s", __func__, error->message);
    g_error_free(error);
  }
  if (p->error != NULL) {
    g_error_free(p->error);
  }
  if likely (error == NULL) {
    should (mtx_unlock_e(&p->mtx, &error) == thrd_success) otherwise {
      g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_ERROR,
            "%s: %s", __func__, error->message);
      g_error_free(error);
    }
  }
  mtx_destroy(&p->mtx);
}


/**
 * @memberof Process
 * @private
 * @brief Search for a executable in the `PATH` environment variable, with
 *        `selfpath` avoided.
 *
 * @param file name of the executable
 * @param selfpath path to be avoided when searching in `PATH`
 * @param[out] error a return location for a GError [optional]
 * @return the full path to the executable, or NULL
 */
static char *Process__search_executable (
    const char *file, const char *selfpath, GError **error) {
  bool would_loop = false;
  char *ret = NULL;

  char *execname = g_path_get_basename(file);
  gchar **search_paths = g_strsplit(g_getenv("PATH"), ":", 0);

  for (int i = 0; search_paths[i] != NULL; i++) {
    gchar *execpath = g_build_filename(search_paths[i], execname, NULL);

    if (!g_path_is_absolute(execpath)) {
      gchar *abspath = g_canonicalize_filename(search_paths[i], NULL);
      g_free(execpath);
      execpath = abspath;
    }

    if (g_file_test(execpath, G_FILE_TEST_IS_SYMLINK)) {
      gchar *realpath = g_file_read_link(execpath, NULL);
      g_free(execpath);
      execpath = realpath;
    }

    if (g_file_test(execpath, G_FILE_TEST_IS_EXECUTABLE)) {
      if (strcmp(execpath, selfpath) != 0) {
        ret = execpath;
        break;
      } else {
        would_loop = true;
      }
    }

    g_free(execpath);
  }

  g_strfreev(search_paths);
  g_free(execname);

  should (ret != NULL) otherwise {
    g_set_error_literal(
      error, DFCC_SPAWN_ERROR, 0,
      would_loop ? "Would call dfcc recursively!" :
        "Cannot find a suitable executable file");
  }

  return ret;
}


int Process_init (
    struct Process *p, gchar **argv, gchar **envp, const char *selfpath,
    ProcessOnchangeCallback onchange, void *userdata, GError **error) {
  if (p != NULL) {
    return_if_fail(
      mtx_init_e(&p->mtx, mtx_plain, error) == thrd_success
    ) 255;
  }

  bool free_argv = false;
  if (selfpath != NULL && strchr(argv[0], '/') == NULL) {
    free_argv = true;
    argv = g_memdup(argv, (g_strv_length(argv) + 1) * sizeof(gchar *));
    argv[0] = Process__search_executable(argv[0], selfpath, error);
    should (argv[0] != NULL) otherwise {
      g_free(argv);
      if (p != NULL) {
        mtx_destroy(&p->mtx);
      }
      return 128;
    }
  }
  gchar **envp_protected =
    g_environ_setenv(g_strdupv(envp), DFCC_LOOP_DETECTION_ENV, "1", TRUE);

  int exit_status = 0;

  do_once {
    GSpawnFlags search_path = selfpath == NULL ? G_SPAWN_SEARCH_PATH : 0;
    // Spawn child process.
    if (p == NULL) {
      should (g_spawn_sync(
          NULL, argv, envp_protected,
          search_path | G_SPAWN_LEAVE_DESCRIPTORS_OPEN,
          NULL, NULL, NULL, NULL, &exit_status, error)) otherwise {
        exit_status = 255;
        break;
      }
      if (!g_spawn_check_exit_status(exit_status, error)) {
        if ((*error)->domain == G_SPAWN_EXIT_ERROR) {
          exit_status = (*error)->code;
        } else {
          exit_status = 254;
        }
      }
    } else {
      should (g_spawn_async_with_pipes(
          NULL, argv, envp_protected,
          search_path | G_SPAWN_DO_NOT_REAP_CHILD |
            G_SPAWN_LEAVE_DESCRIPTORS_OPEN,
          NULL, NULL, &p->pid, &p->stdin, NULL, NULL, error) // temp
      ) otherwise {
        mtx_destroy(&p->mtx);
        exit_status = 255;
        break;
      }

      p->stopped = false;
      p->error = NULL;
      p->onchange = onchange;
      p->userdata = userdata;
    }
  }

  g_strfreev(envp_protected);
  if (free_argv) {
    g_free(argv[0]);
    g_free(argv);
  }

  if (p != NULL) {
    g_child_watch_add(p->pid, Process_child_watch_cb, p);
  }

  return exit_status;
}
