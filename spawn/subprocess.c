#include <stdbool.h>
#include <string.h>

#include <glib.h>

#include <macro.h>

#include "../version.h"
#include "subprocess.h"


GQuark DFCC_SPAWN_ERROR;


static void __attribute__ ((constructor)) DFCC_SPAWN_ERROR_init (void) {
  DFCC_SPAWN_ERROR = g_quark_from_static_string("dfcc-spawn-error-quark");
}


extern inline void Subprocess_destroy (struct Subprocess *spawn);


static void Subprocess_child_watch_cb (GPid pid, gint status, gpointer user_data) {
  struct Subprocess *spawn = (struct Subprocess *) user_data;
  spawn->stopped = true;
  if (g_spawn_check_exit_status(status, &spawn->error)) {
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
          "Child %" G_PID_FORMAT " exited normally", pid);
  } else {
    g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG,
          "Child %" G_PID_FORMAT " exited abnormally: %s",
          pid, spawn->error->message);
  }
  if (spawn->onfinish != NULL) {
    spawn->onfinish(spawn, spawn->userdata);
  }
}


/**
 * @memberof Subprocess
 * @private
 * @brief Search for a executable in the `PATH` environment variable, with
 *        `selfpath` avoided.
 *
 * @param file name of the executable
 * @param selfpath path to be avoided when searching in `PATH`
 * @param[out] error a return location for a GError [optional]
 * @return the full path to the executable, or NULL
 */
static char *Subprocess__search_executable (
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


int Subprocess_init (
    struct Subprocess *p, gchar **argv, gchar **envp,
    const char *selfpath, GError **error) {

  GSpawnFlags search_path = selfpath == NULL ? G_SPAWN_SEARCH_PATH : 0;
  gchar **argv_searched;
  if (selfpath == NULL) {
    argv_searched = argv;
  } else {
    argv_searched =
      g_memdup(argv, (g_strv_length(argv) + 1) * sizeof(gchar *));
    argv_searched[0] = Subprocess__search_executable(argv[0], selfpath, error);
    should (argv_searched[0] != NULL) otherwise {
      g_free(argv_searched);
      return 128;
    }
  }
  gchar **envp_protected =
    g_environ_setenv(g_strdupv(envp), DFCC_LOOP_DETECTION_ENV, "1", TRUE);

  int exit_status = 0;

  do_once {
    // Spawn child process.
    if (p == NULL) {
      should (g_spawn_sync(
          NULL, argv_searched, envp_protected, search_path |
          G_SPAWN_LEAVE_DESCRIPTORS_OPEN, NULL, NULL,
          NULL, NULL, &exit_status, error)) otherwise {
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
          NULL, argv_searched, envp_protected, search_path |
          G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_LEAVE_DESCRIPTORS_OPEN, NULL, NULL,
          &p->pid, &p->stdin, &p->stdout, &p->stderr, error)
      ) otherwise {
        exit_status = 255;
        break;
      }

      g_child_watch_add(p->pid, Subprocess_child_watch_cb, p);
      p->stopped = false;
      p->error = NULL;
      p->onfinish = NULL;
      p->userdata = NULL;
    }
  }

  g_free(envp_protected);
  if (selfpath != NULL) {
    g_free(argv_searched[0]);
    g_free(argv_searched);
  }

  return exit_status;
}
