#ifndef MORESTRING_H
#define MORESTRING_H
/**
 * @addtogroup Common Common
 * @{
 * @defgroup String String
 * @brief String related functions
 * @{
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>


#define CONSTSTR(s) (s), strlen(s)
#define strscmp(str1, str2) strncmp(str1, str2, strlen(str2))


/**
 * @brief Remove the trailing whitespace of a string.
 *
 * @param s a string
 * @param n the length of `s`
 * @return `s`
 */
inline char *strnrstrip (char *s, size_t n) {
  char *end = s + n - 1;
  while (end >= s && isspace(*end)) {
    end--;
  }
  *(end + 1) = '\0';
  return s;
}


/**
 * @brief Remove the trailing whitespace of a string.
 *
 * @param s a string
 * @return `s`
 */
inline char *strrstrip (char *s) {
  return strnrstrip(s, strlen(s));
}


/**
 * @brief Remove the leading whitespace of a string.
 *
 * @param s a string
 * @return `s`
 */
inline char *strlstrip (char *s) {
  while (isspace(*s)) {
    s++;
  }
  return s;
}


/**
 * @brief Remove the leading and trailing whitespace of a string.
 *
 * @param s a string
 * @return `s`
 */
inline char *strstrip (char *s) {
  strrstrip(s);
  return strlstrip(s);
}


/**
 * @brief Duplicate a string and return its length.
 *
 * @param s a string
 * @param[out] n a return location for the length of `s`
 * @return newly-allocated string with `s` duplicated
 */
inline char *strlendup (const char *s, size_t *n) {
  *n = strlen(s);
  char *t = g_malloc(*n + 1);
  memcpy(t, s, *n + 1);
  return t;
}


/**
 * @}
 * @}
 */
#endif /* MORESTRING_H */
