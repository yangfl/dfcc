#ifndef HOOKFS_SERIALIZER_H
#define HOOKFS_SERIALIZER_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "common/macro.h"
#include "common/simplestring.h"
#include "limit.h"


//! @memberof Serializer
typedef int (*Serializer__printf_t)
  (void * restrict, const char * restrict, ...)
  __attribute__((format(printf, 2, 3)));
//! @memberof Serializer
typedef size_t (*Serializer__write_t)
  (const void * restrict, size_t, size_t, void * restrict);
//! @memberof Serializer
typedef int (*Serializer__scanf_t)
  (void * restrict, const char * restrict, ...)
  __attribute__((format(scanf, 2, 3)));
//! @memberof Serializer
typedef size_t (*Serializer__read_t)
  (void * restrict, size_t, size_t, void * restrict);
//! @memberof Serializer
typedef SimpleString__size_t Serializer__size_t;


//! @ingroup Hookfs
struct Serializer {
  void *ostream;
  Serializer__printf_t printf;
  Serializer__write_t write;
  void *istream;
  Serializer__scanf_t scanf;
  Serializer__read_t read;
};


/**
 * @memberof Serializer
 * @brief Serializes a token.
 *
 * @param serdes a Serializer
 * @param data data to be serialized
 * @param len length of `data`
 */
inline int serialize (struct Serializer *serdes, const void *data, Serializer__size_t len) {
  int ret = serdes->write(&len, sizeof(len), 1, serdes->ostream);
  should (ret >= 0) otherwise {
    return ret;
  }
  ret += serdes->write(data, len, 1, serdes->ostream);
  return ret;
}

//! @memberof Serializer
#define serialize_printf(serdes, fmt, ...) { \
  char buf[Hookfs_MAX_TOKEN_LEN]; \
  SimpleString__size_t len = snprintf(buf, sizeof(buf), (fmt), __VA_ARGS__); \
  serialize((serdes)->ostream, buf, len); \
}

/**
 * @memberof Serializer
 * @brief Serializes a null-terminated string.
 *
 * @param serdes a Serializer
 * @param str a null-terminated string
 */
inline int serialize_string (struct Serializer *serdes, const char *str) {
  return serialize(serdes, str, strlen(str) + 1);
}

/**
 * @memberof Serializer
 * @brief Serializes a literal string.
 *
 * @param serdes a Serializer
 * @param str a literal string
 */
#define serialize_literal(serdes, str) \
  serialize((serdes), (str), sizeof(str))

/**
 * @memberof Serializer
 * @brief Serializes a numerical variable.
 *
 * @param serdes a Serializer
 * @param num a number
 */
#define serialize_numerical(serdes, num) \
  serialize((serdes), &(num), sizeof(num))

//! @memberof Serializer
int serialize_strv (struct Serializer *serdes, char * const *data);

//! @memberof Serializer
inline int serialize_end (struct Serializer *serdes) {
  SimpleString__size_t len = 0;
  return serdes->write(&len, sizeof(len), 1, serdes->ostream);
}

//! @memberof Serializer
void *deserialize (struct Serializer *serdes, void *buf, size_t size, size_t *read);

//! @memberof Serializer
inline void *deserialize_new (struct Serializer *serdes, size_t *read) {
  return deserialize(serdes, NULL, 0, read);
}

//! @memberof Serializer
char *deserialize_string (struct Serializer *serdes, char *buf, size_t size, size_t *read);


#endif /* HOOKFS_SERIALIZER_H */
