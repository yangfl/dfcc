#include <alloca.h>  // alloca
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <macro.h>

#include "serializer.h"


extern inline int serialize (struct SerializerIOFuncs *iofuncs, const void *data, size_t len);
extern inline int serialize_string (struct SerializerIOFuncs *iofuncs, const char *data);
extern inline int serialize_end (struct SerializerIOFuncs *iofuncs);
extern inline void *deserialize_new (struct SerializerIOFuncs *iofuncs, size_t *read);


int serialize_strv (struct SerializerIOFuncs *iofuncs, char * const *data) {
  char buf[Hookfs_MAX_TOKEN_LEN];
  size_t buf_i = 0;
  for (int i = 0; data[i] != NULL; i++) {
    size_t data_i_len = strlen(data[i]);
    buf_i += snprintf(buf + buf_i, sizeof(buf) - buf_i, iofuncs->ostream, "%zu_", data_i_len);
    memcpy(buf + buf_i, data[i], data_i_len);
    buf_i += data_i_len;
  }

  size_t buf_len = buf_i + 1;
  int ret = iofuncs->printf(iofuncs->ostream, "%zd_", buf_len);
  should (ret >= 0) otherwise {
    return ret;
  }
  ret += iofuncs->write(buf, buf_len, 1, iofuncs->ostream);
  return ret;
}


void *deserialize (struct SerializerIOFuncs *iofuncs, void *buf, size_t size, size_t *read) {
  bool buf_is_null = buf == NULL;
  bool buf_too_short = false;

  size_t len_to_read;
  char delimiter;
  should (iofuncs->scanf(iofuncs->istream, "%zu%c", &len_to_read, &delimiter) == 2) otherwise {
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
  size_t data_read = iofuncs->read(buf, 1, len_to_read, iofuncs->istream);
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


char *deserialize_string (struct SerializerIOFuncs *iofuncs, char *buf, size_t size, size_t *read) {
  size_t read_;
  char *ret = deserialize(iofuncs, buf, size - 1, &read_);
  should (ret != NULL) otherwise return NULL;
  ret[read_] = '\0';
  if (read != NULL) {
    *read = read_;
  }
  return ret;
}
