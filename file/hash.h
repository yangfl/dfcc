#ifndef DFCC_FILE_HASH_H
#define DFCC_FILE_HASH_H

#include <string.h>

#include <glib.h>
#include <xxhash.h>

#include <hexstring.h>
#include <macro.h>


/**
 * @ingroup File
 * @brief Contains the hash value of given data.
 */
struct FileHash {
  /**
   * @private
   * @brief The hash value.
   */
  XXH64_hash_t hash;
};


/**
 * @ingroup File
 * @brief The length of the printable representation of a FileHash.
 */
#define FileHash_STRLEN (2 * sizeof(struct FileHash))
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
 * It can be passed to `g_hash_table_new()` as the `hash_func` parameter, when using non-`NULL` pointers to FileHash as keys in a `GHashTable`.
 *
 * @param v a pointer to a FileHash key
 * @return a hash value corresponding to the key
 */
#define FileHash_hash g_int64_hash
/**
 * @memberof FileHash
 * @brief Frees associated resources of a FileHash.
 *
 * @param hash a FileHash
 */
#define FileHash_destroy(...)
/**
 * @memberof FileHash
 * @brief Frees a FileHash and associated resources.
 *
 * @param hash a FileHash
 */
#define FileHash_free g_free

/**
 * @memberof FileHash
 * @brief Convert `hash` to its printable representation into `s`.
 *
 * @param hash a FileHash
 * @param[out] s a buffer of length at least @ref FileHash_STRLEN [nullable]
 * @return `s` if non-null, otherwise a newly-allocated buffer [transfer-full]
 */
inline char *FileHash_to_string (struct FileHash *hash, char *s) {
  if (s == NULL) {
    s = g_malloc(FileHash_STRLEN + 1);
  }
  buf2hex(s, &hash->hash, sizeof(hash->hash));
  s[FileHash_STRLEN] = '\0';
  return s;
}

/**
 * @memberof FileHash
 * @brief Initializes a FileHash with the existing FileHash `orig`.
 *
 * @param hash a FileHash
 * @param orig the existing FileHash
 * @return 0 if success, otherwize nonzero
 */
inline int FileHash_init_copy (
    struct FileHash *hash, const struct FileHash *orig) {
  memcpy(hash, orig, sizeof(struct FileHash));
  return 0;
}

/**
 * @memberof FileHash
 * @brief Makes a copy of `hash`.
 *
 * @param hash a FileHash
 * @return a new FileHash
 */
inline struct FileHash *FileHash_new_copy (const struct FileHash *hash) {
  return g_memdup(hash, sizeof(struct FileHash));
}

/**
 * @memberof FileHash
 * @brief Initializes a FileHash by hashing the string `s`.
 *
 * @param hash a FileHash
 * @param s the string
 * @return 0 if success, otherwize nonzero
 */
inline int FileHash_init_from_string (struct FileHash *hash, const char *s) {
  char *s_end;
  hash->hash = strtoull(s, &s_end, 16);
  if (s_end - s < FileHash_STRLEN) {
    return 1;
  }
  return 0;
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
inline int FileHash_init_from_buf (
    struct FileHash *hash, const void* buf, size_t size) {
  hash->hash = XXH64(buf, size, 0);
  return 0;
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
int FileHash_init_from_file (
    struct FileHash *hash, const char* path, GError **error);

/**
 * @memberof FileHash
 * @brief Creates a new FileHash by hashing the content of the file `path`.
 *
 * @param path path to the file
 * @param[out] error a return location for a GError [optional]
 * @return a new FileHash if success, otherwize NULL
 */
inline struct FileHash *FileHash_new_from_file (
    const char* path, GError **error) {
  struct FileHash *hash = g_malloc(sizeof(struct FileHash));
  should (FileHash_init_from_file(hash, path, error) == 0) otherwise {
    g_free(hash);
    return NULL;
  }
  return hash;
}


#endif /* DFCC_FILE_HASH_H */
