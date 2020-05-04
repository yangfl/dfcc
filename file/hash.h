#ifndef DFCC_FILE_HASH_H
#define DFCC_FILE_HASH_H

#include <string.h>

#include <glib.h>
#include <xxhash.h>

#include <hexstring.h>
#include <macro.h>


/**
 * @ingroup File
 * @class FileHash
 * @brief Contains the hash value of given data. Always be uint64_t.
 */
typedef unsigned long long FileHash;


/**
 * @ingroup File
 * @brief The length of the printable representation of a FileHash.
 */
#define FileHash_STRLEN (2 * sizeof(FileHash))
/**
 * @memberof FileHash
 * @brief Compares the two FileHash values being pointed to and returns `TRUE`
 *        if they are equal.
 *
 * It can be passed to `g_hash_table_new()` as the `key_equal_func` parameter,
 * when using non-`NULL` pointers to FileHash as keys in a `GHashTable`.
 *
 * @param v1 a pointer to a FileHash key
 * @param v2 a pointer to a FileHash key to compare with `v1`
 * @return `TRUE` if the two keys match
 */
#define FileHash_equal g_int64_equal
/**
 * @memberof FileHash
 * @brief Converts a pointer to a FileHash to a hash value.
 *
 * It can be passed to `g_hash_table_new()` as the `hash_func` parameter, when
 * using non-`NULL` pointers to FileHash as keys in a `GHashTable`.
 *
 * @param v a pointer to a FileHash key
 * @return a hash value corresponding to the key
 */
#define FileHash_hash g_int64_hash

/**
 * @memberof FileHash
 * @brief Convert `hash` to its printable representation into a buffer.
 *
 * @param hash a FileHash
 * @param[out] s a buffer of length at least @ref FileHash_STRLEN [nullable]
 * @return `s` if non-null, otherwise a newly-allocated buffer [transfer-full]
 */
inline char *FileHash_to_buf (FileHash hash, char s[FileHash_STRLEN]) {
  if (s == NULL) {
    s = g_malloc(FileHash_STRLEN);
  }
  buf2hex(s, &hash, sizeof(hash));
  return s;
}

/**
 * @memberof FileHash
 * @brief Convert `hash` to its printable representation into a string.
 *
 * @param hash a FileHash
 * @param[out] s a buffer of length at least @ref FileHash_STRLEN + 1 [nullable]
 * @return `s` if non-null, otherwise a newly-allocated buffer [transfer-full]
 */
inline char *FileHash_to_string (FileHash hash, char s[FileHash_STRLEN + 1]) {
  if (s == NULL) {
    s = g_malloc(FileHash_STRLEN + 1);
  }
  FileHash_to_buf(hash, s);
  s[FileHash_STRLEN] = '\0';
  return s;
}

/**
 * @memberof FileHash
 * @brief Initializes a FileHash by hashing the string `s`.
 *
 * @param hash a FileHash
 * @param s the string
 * @return 0 if success, otherwize nonzero
 */
inline FileHash FileHash_from_string (const char *s) {
  char *s_end;
  FileHash hash = strtoull(s, &s_end, 16);
  if unlikely (s_end - s < FileHash_STRLEN) {
    return 0;
  }
  return hash;
}

/**
 * @memberof FileHash
 * @brief Initializes a FileHash by hashing the content of the buffer `buf`.
 *
 * @param hash a FileHash
 * @param buf the buffer
 * @param size the length of buffer
 * @return 0 if success, otherwize nonzero
 */
inline FileHash FileHash_from_buf (const void* buf, size_t size) {
  FileHash hash = XXH64(buf, size, 0);
  if unlikely (hash == 0) {
    hash = 1;
  }
  return hash;
}

/**
 * @memberof FileHash
 * @brief Initializes a FileHash by hashing the content of the file `path`.
 *
 * @param hash a FileHash
 * @param path path to the file
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
FileHash FileHash_from_file (const char* path, GError **error);


#endif /* DFCC_FILE_HASH_H */
