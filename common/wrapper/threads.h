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

#undef WRAP_THREADS_GERROR


/**@}*/

#endif /* DFCC_WRAPPER_THREADS_H */
