#ifndef DFCC_SPAWN_SUBPROCESS_H
#define DFCC_SPAWN_SUBPROCESS_H

#include <stdbool.h>

#include <glib.h>


extern GQuark DFCC_SPAWN_ERROR;


struct Subprocess {
  GPid pid;
  gint stdin;
  gint stdout;
  gint stderr;
  bool stopped;
  GError *error;
  void (*onfinish)(struct Subprocess *spawn, void *userdata);
  void *userdata;
};


inline void Subprocess_destroy (struct Subprocess *p) {
  g_spawn_close_pid(p->pid);
  g_error_free(p->error);
}

int Subprocess_init (
    struct Subprocess *p, gchar **argv, gchar **envp,
    const char *selfpath, GError **error);


#endif /* DFCC_SPAWN_SUBPROCESS_H */
