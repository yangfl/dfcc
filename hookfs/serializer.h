#ifndef HOOKFS_SERIALIZER_H
#define HOOKFS_SERIALIZER_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <simplestring.h>

/**
 * @addtogroup Hookfs
 * @{
 */


#define Hookfs_MAX_TOKEN_LEN 8192


typedef int (*SerializerIOFuncs__printf_t)
  (void * restrict, const char * restrict, ...)
  __attribute__((format(printf, 2, 3)));
typedef int (*SerializerIOFuncs__puts_t)
  (const char * restrict, void * restrict);
typedef int (*SerializerIOFuncs__write_t)
  (const void * restrict, size_t, size_t, void * restrict);
typedef int (*SerializerIOFuncs__scanf_t)
  (void * restrict, const char * restrict, ...)
  __attribute__((format(scanf, 2, 3)));
typedef int (*SerializerIOFuncs__read_t)
  (const void * restrict, size_t, size_t, void * restrict);


struct SerializerIOFuncs {
  void *ostream;
  SerializerIOFuncs__printf_t printf;
  SerializerIOFuncs__puts_t puts;
  SerializerIOFuncs__write_t write;
  void *istream;
  SerializerIOFuncs__scanf_t scanf;
  SerializerIOFuncs__read_t read;
};


/**
 * @brief Serializes a token.
 *
 * @param iofuncs->ostream output FILE iofuncs->ostream
 * @param data data to be serialized
 * @param len length of `data`
 */
inline void serialize (struct SerializerIOFuncs *iofuncs, const void *data, size_t len) {
  iofuncs->printf(iofuncs->ostream, "%zu_", len);
  iofuncs->write(data, len, 1, iofuncs->ostream);
}

#define serialize_printf(iofuncs, fmt, ...) { \
  char buf[Hookfs_MAX_TOKEN_LEN]; \
  size_t len = snprintf(buf, sizeof(buf), (fmt), __VA_ARGS__); \
  serialize((iofuncs)->ostream, buf, len); \
}

/**
 * @brief Serializes a null-terminated string.
 *
 * @param iofuncs->ostream output FILE iofuncs->ostream
 * @param data a null-terminated string
 */
inline void serialize_string (struct SerializerIOFuncs *iofuncs, const char *data) {
  serialize(iofuncs->ostream, data, strlen(data));
}

void serialize_strv (struct SerializerIOFuncs *iofuncs, char * const *data);

inline void serialize_end (struct SerializerIOFuncs *iofuncs) {
  iofuncs->puts("0\n", iofuncs->ostream);
}

void *deserialize (struct SerializerIOFuncs *iofuncs, void *buf, size_t size, size_t *read);

inline void *deserialize_new (struct SerializerIOFuncs *iofuncs, size_t *read) {
  return deserialize(iofuncs->istream, NULL, 0, read);
}

char *deserialize_string (struct SerializerIOFuncs *iofuncs, char *buf, size_t size, size_t *read);


/**@}*/


#endif /* HOOKFS_SERIALIZER_H */
