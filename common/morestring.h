#ifndef MORESTRING_H
#define MORESTRING_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>


#define CONSTSTR(s) s, strlen(s)
#define strscmp(str1, str2) strncmp(str1, str2, strlen(str2))


inline char *strnrstrip (char *s, size_t n) {
  char *end = s + n - 1;
  while (end >= s && isspace(*end)) {
    end--;
  }
  *(end + 1) = '\0';
  return s;
}


inline char *strrstrip (char *s) {
  return strnrstrip(s, strlen(s));
}


inline char *strlstrip (char *s) {
  while (isspace(*s)) {
    s++;
  }
  return s;
}


inline char *strstrip (char *s) {
  strrstrip(s);
  return strlstrip(s);
}


inline char *strlendup (const char *s, size_t *n) {
  *n = strlen(s);
  char *t = g_malloc(*n + 1);
  memcpy(t, s, *n + 1);
  return t;
}


#endif /* MORESTRING_H */
