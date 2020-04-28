#include <stddef.h>
#include <string.h>
#include <stdio.h>
/**
 * @addtogroup Hookfs
 * @{
 */


#define Hookfs_MAX_TOKEN_LEN 8192


/**
 * @brief Serializes a token.
 *
 * @param stream output FILE stream
 * @param data data to be serialized
 * @param len length of `data`
 */
inline void serialize (FILE *stream, const void *data, size_t len) {
  fprintf(stream, "%zd_", len);
  fwrite(data, len, 1, stream);
}

#define serialize_printf(stream, fmt, ...) { \
  char buf[Hookfs_MAX_TOKEN_LEN]; \
  size_t len = snprintf(buf, sizeof(buf), (fmt), __VA_ARGS__); \
  serialize((stream), buf, len); \
}

/**
 * @brief Serializes a null-terminated string.
 *
 * @param stream output FILE stream
 * @param data a null-terminated string
 */
inline void serialize_string (FILE *stream, const char *data) {
  serialize(stream, data, strlen(data));
}

void serialize_strv (FILE *stream, char * const *data);

inline void serialize_end (FILE *stream) {
  fputs("0\n", stream);
}

void *deserialize (FILE *stream, void *buf, size_t size, size_t *read);

inline void *deserialize_new (FILE *stream, size_t *read) {
  return deserialize(stream, NULL, 0, read);
}

char *deserialize_string (FILE *stream, char *buf, size_t size, size_t *read);


/**@}*/
