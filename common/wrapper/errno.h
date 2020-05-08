#ifndef DFCC_WRAPPER_ERRNO_H
#define DFCC_WRAPPER_ERRNO_H
/**
 * @ingroup Wrapper
 * @defgroup Errno Errno
 * @brief Set GError accroding to errno
 * @{
 */

#include <glib.h>


/**
 * @brief Set GError accroding to errno
 *
 * Does nothing if `err` is `NULL`; if `err` is non-`NULL`, then `*err` must be
 * `NULL`. A new GError is created and assigned to `*err`.
 *
 * @param err a return location for a GError [out callee-allocates][optional]
 * @param domain error domain
 * @param format printf()-style format
 */
void g_set_error_errno (GError **error, GQuark domain, const char *format);


/**@}*/

#endif /* DFCC_WRAPPER_ERRNO_H */
