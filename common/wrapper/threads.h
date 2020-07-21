#ifndef DFCC_WRAPPER_THREADS_H
#define DFCC_WRAPPER_THREADS_H
/**
 * @ingroup Wrapper
 * @defgroup Threads Threads
 * @brief Wrapped threads.h
 * @{
 */

#include <threads.h>

#include <glib.h>

#include "common/macro.h"
#include "errno.h"


void threads_set_error (GError **error, const char *format);

#define WRAP_THREADS_GERROR(type, func, params, args, test, msg) \
inline type func ## _e params { \
  type ret = func args; \
  should (ret test) otherwise { \
    g_set_error_errno(error, G_THREAD_ERROR, msg ": %s"); \
  } \
  return ret; \
}

WRAP_THREADS_GERROR(
  int, mtx_init, (mtx_t *mutex, int type, GError **error),
  (mutex, type), == thrd_success, "Failed to initialise mutex")
WRAP_THREADS_GERROR(
  int, mtx_lock, (mtx_t *mutex, GError **error),
  (mutex), == thrd_success, "Failed to lock mutex")
WRAP_THREADS_GERROR(
  int, mtx_unlock, (mtx_t *mutex, GError **error),
  (mutex), == thrd_success, "Failed to unlock mutex")

WRAP_THREADS_GERROR(
  int, cnd_init, (cnd_t* cond, GError **error),
  (cond), == thrd_success, "Failed to initialise condition")
WRAP_THREADS_GERROR(
  int, cnd_signal, (cnd_t* cond, GError **error),
  (cond), == thrd_success, "Failed to unblock condition")
WRAP_THREADS_GERROR(
  int, cnd_broadcast, (cnd_t* cond, GError **error),
  (cond), == thrd_success, "Failed to broadcast condition")
WRAP_THREADS_GERROR(
  int, cnd_wait, (cnd_t* cond, mtx_t* mutex, GError **error),
  (cond, mutex), == thrd_success, "Failed to wait condition")

#undef WRAP_THREADS_GERROR


/**@}*/

#endif /* DFCC_WRAPPER_THREADS_H */
