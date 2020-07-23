#include <alloca.h>  // alloca
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "common/macro.h"
#include "serializer.h"


extern inline ssize_t Serializer_write (struct Serializer *serdes, const void *buf, size_t count);
extern inline ssize_t Serializer_read (struct Serializer *serdes, void *buf, size_t count);
extern inline ssize_t serialize_string (struct Serializer *serdes, const char *str);
extern inline ssize_t serialize_end (struct Serializer *serdes);
extern inline char deserialize_next (struct Serializer *serdes, int *err);
extern inline ssize_t deserialize_length (struct Serializer *serdes, uint8_t type);
extern inline void *deserialize_new (struct Serializer *serdes, size_t size, size_t *read);


ssize_t serialize_numerical (struct Serializer *serdes, uint64_t num) {
  const uint8_t numerical = MESSAGE_NUMERICAL;
  ssize_t ret = Serializer_write(serdes, &numerical, sizeof(numerical));
  should (ret >= 0) otherwise {
    return ret;
  }
  ssize_t err = Serializer_write(serdes, &num, sizeof(num));
  should (err >= 0) otherwise {
    return err;
  }
  return ret + err;
}


ssize_t serialize_string_len (struct Serializer *serdes, const char *str, uint64_t len) {
  const uint8_t string_ = MESSAGE_STRING;
  ssize_t ret = Serializer_write(serdes, &string_, sizeof(string_));
  should (ret >= 0) otherwise {
    return ret;
  }
  ssize_t err = Serializer_write(serdes, &len, sizeof(len));
  should (err >= 0) otherwise {
    return err;
  }
  ret += err;
  err = Serializer_write(serdes, str, len);
  should (err >= 0) otherwise {
    return err;
  }
  ret += err;
  return ret;
}


ssize_t serialize_strv (struct Serializer *serdes, char * const *data) {
  const uint8_t array = MESSAGE_ARRAY;
  ssize_t ret = Serializer_write(serdes, &array, sizeof(array));
  should (ret >= 0) otherwise {
    return ret;
  }

  for (int i = 0; data[i] != NULL; i++) {
    ssize_t err = serialize_string(serdes, data[i]);
    should (err >= 0) otherwise {
      return err;
    }
    ret += err;
  }

  ssize_t err = serialize_end(serdes);
  should (err >= 0) otherwise {
    return err;
  }
  ret += err;

  return ret;
}


void *deserialize (struct Serializer *serdes, void *buf, size_t size, size_t *read) {
  bool buf_is_null = buf == NULL;

  if (buf_is_null) {
    buf = malloc(size);
    should (buf != NULL) otherwise {
      // error: memory low
      return NULL;
    }
  }

  ssize_t data_read = Serializer_read(serdes, buf, size);
  if (read != NULL) {
    *read = data_read;
  }

  should (data_read == size) otherwise {
    // error: length mismatch
    if (buf_is_null) {
      free(buf);
    }
    return NULL;
  }
  return buf;
}
