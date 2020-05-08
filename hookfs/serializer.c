#include <alloca.h>  // alloca
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "common/macro.h"
#include "serializer.h"


extern inline int serialize (struct Serializer *serdes, const void *data, Serializer__size_t len);
extern inline int serialize_string (struct Serializer *serdes, const char *data);
extern inline int serialize_end (struct Serializer *serdes);
extern inline void *deserialize_new (struct Serializer *serdes, size_t *read);


int serialize_strv (struct Serializer *serdes, char * const *data) {
  char buf[Hookfs_MAX_TOKEN_LEN];

  Serializer__size_t buf_i = 0;
  for (int i = 0; data[i] != NULL; i++) {
    Serializer__size_t data_i_len = strlen(data[i]);
    memcpy(buf + buf_i, &data_i_len, sizeof(data_i_len));
    buf_i += sizeof(data_i_len);
    memcpy(buf + buf_i, data[i], data_i_len);
    buf_i += data_i_len;
  }
  Serializer__size_t data_end = 0;
  memcpy(buf + buf_i, &data_end, sizeof(data_end));
  buf_i += sizeof(data_end);

  return serialize(serdes, buf, buf_i + 1);
}


void *deserialize (struct Serializer *serdes, void *buf, size_t size, size_t *read) {
  bool buf_is_null = buf == NULL;
  bool buf_too_short = false;

  Serializer__size_t len_to_read;
  should (serdes->read(&len_to_read, sizeof(len_to_read), 1, serdes->istream) == sizeof(len_to_read)) otherwise {
    // error: wrong format
    return NULL;
  }
  should (len_to_read <= Hookfs_MAX_TOKEN_LEN) otherwise {
    // error: too long
    return NULL;
  }
  should (!buf_is_null && len_to_read <= size) otherwise {
    // warning: buf too short
    buf = alloca(len_to_read);
  }

  if (buf_is_null) {
    buf = malloc(len_to_read);
    should (buf != NULL) otherwise {
      // error: memory low
      return NULL;
    }
  }
  size_t data_read = serdes->read(buf, 1, len_to_read, serdes->istream);
  should (data_read == len_to_read) otherwise {
    // error: length mismatch
    if (buf_is_null) {
      free(buf);
    }
    return NULL;
  }

  if (read != NULL) {
    *read = data_read;
  }
  if (buf_too_short) {
    return NULL;
  }
  return buf;
}


char *deserialize_string (struct Serializer *serdes, char *buf, size_t size, size_t *read) {
  size_t read_;
  char *ret = deserialize(serdes, buf, size - 1, &read_);
  should (ret != NULL) otherwise return NULL;
  ret[read_] = '\0';
  if (read != NULL) {
    *read = read_;
  }
  return ret;
}
