#ifndef HOOKFS_SERIALIZER_H
#define HOOKFS_SERIALIZER_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#include "common/macro.h"
#include "limit.h"


//! @memberof Serializer
typedef ssize_t (*Serializer__write_t) (void *, const void *, size_t);
//! @memberof Serializer
typedef ssize_t (*Serializer__read_t) (void *, void *, size_t);


//! @ingroup Hookfs
struct Serializer {
  void *ostream;
  Serializer__write_t write;
  void *istream;
  Serializer__read_t read;
};


//! @memberof Serializer
inline ssize_t Serializer_write (struct Serializer *serdes, const void *buf, size_t count) {
  return serdes->write(serdes->ostream, buf, count);
}

//! @memberof Serializer
inline ssize_t Serializer_read (struct Serializer *serdes, void *buf, size_t count) {
  return serdes->read(serdes->istream, buf, count);
}


enum {
  MESSAGE_END = 0,
  MESSAGE_ARRAY,
  MESSAGE_NUMERICAL,
  MESSAGE_STRING,
};


__attribute__((__packed__))
struct Message {
  uint8_t type;
  uint64_t num;
};


/**
 * @memberof Serializer
 * @brief Serializes a numerical variable.
 *
 * @param serdes a Serializer
 * @param num a number
 */
ssize_t serialize_numerical (struct Serializer *serdes, uint64_t num);

/**
 * @memberof Serializer
 * @brief Serializes a string.
 *
 * @param serdes a Serializer
 * @param str string to be serialized
 * @param len length of `str`
 */
ssize_t serialize_string_len (struct Serializer *serdes, const char *str, uint64_t len);

//! @memberof Serializer
#define serialize_printf(serdes, fmt, ...) { \
  char buf[Hookfs_MAX_TOKEN_LEN]; \
  size_t len = snprintf(buf, sizeof(buf), (fmt), __VA_ARGS__); \
  serialize_string_len((serdes)->ostream, buf, len); \
}

/**
 * @memberof Serializer
 * @brief Serializes a null-terminated string.
 *
 * @param serdes a Serializer
 * @param str a null-terminated string
 */
inline ssize_t serialize_string (struct Serializer *serdes, const char *str) {
  return serialize_string_len(serdes, str, strlen(str) + 1);
}

/**
 * @memberof Serializer
 * @brief Serializes a literal string.
 *
 * @param serdes a Serializer
 * @param str a literal string
 */
#define serialize_literal(serdes, str) \
  serialize_string_len((serdes), (str), sizeof(str))

//! @memberof Serializer
ssize_t serialize_strv (struct Serializer *serdes, char * const *data);

//! @memberof Serializer
inline ssize_t serialize_end (struct Serializer *serdes) {
  const uint8_t end = MESSAGE_END;
  return Serializer_write(serdes, &end, sizeof(end));
}

#define fortoken(t, serdes, err) for (uint8_t t; t = deserialize_next(serdes, err);)

//! @memberof Serializer
inline char deserialize_next (struct Serializer *serdes, int *err) {
  uint8_t type;
  ssize_t ret = Serializer_read(serdes, &type, sizeof(type));
  should (ret >= 0) otherwise {
    *err = (int) ret;
  }
  return type;
}

//! @memberof Serializer
inline ssize_t deserialize_length (struct Serializer *serdes, uint8_t type) {
  switch (type) {
    case MESSAGE_NUMERICAL:
      return sizeof(uint64_t);
    case MESSAGE_STRING: {
      uint64_t len;
      ssize_t ret = Serializer_read(serdes, &len, sizeof(len));
      should (ret >= 0) otherwise {
        return ret;
      }
      return len;
    }
    default:
      return -2;
  }
}

//! @memberof Serializer
void *deserialize (struct Serializer *serdes, void *buf, size_t size, size_t *read);

//! @memberof Serializer
inline void *deserialize_new (struct Serializer *serdes, size_t size, size_t *read) {
  return deserialize(serdes, NULL, size, read);
}

#endif /* HOOKFS_SERIALIZER_H */
