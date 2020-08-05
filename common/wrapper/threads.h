#ifndef DFCC_WRAPPER_THREADS_H
#define DFCC_WRAPPER_THREADS_H
/**
 * @ingroup Wrapper
 * @defgroup Threads Threads
 * @brief Wrapped threads.h
 * @{
 */

#include <stdbool.h>
#include <threads.h>

#include <glib.h>

#include "common/macro.h"
#include "errno.h"


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


#define CRITICAL_SECTIONS_START(mutex, name) \
  bool __critical_ ## name = true; \
  { \
    GError *error = NULL; \
    should (mtx_lock_e(&p->mtx, &error) == thrd_success) otherwise { \
      __critical_ ## name = false; \
      g_log("threads", G_LOG_LEVEL_ERROR, "%s: %s", __func__, error->message); \
      g_error_free(error); \
    } \
  }

#define CRITICAL_SECTIONS_END(mutex, name) \
  if (__critical_ ## name) { \
    GError *error = NULL; \
    should (mtx_unlock_e(&p->mtx, &error) == thrd_success) otherwise { \
      g_log("threads", G_LOG_LEVEL_ERROR, "%s: %s", __func__, error->message); \
      g_error_free(error); \
    } \
  }


/**@}*/

#endif /* DFCC_WRAPPER_THREADS_H */
