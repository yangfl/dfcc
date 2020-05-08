#ifndef ATOMIC_COUNT_H
#define ATOMIC_COUNT_H
/**
 * @ingroup Common Common
 * @defgroup AtomicCount Atomic Count
 * @brief Atmoic count
 * @{
 */

#include <stdatomic.h>
#include <stdbool.h>


inline bool count_dec (atomic_int *counter) {
  int newval = --*counter;
  if (newval < 0) {
    ++*counter;
    return false;
  }
  return true;
}


/**@}*/

#endif /* ATOMIC_COUNT_H */
