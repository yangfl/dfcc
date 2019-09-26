#ifndef DFCC_FILE_HASH_H
#define DFCC_FILE_HASH_H

#include <string.h>

#include <glib.h>
#include <xxhash.h>

#include <hexstring.h>
#include <macro.h>


struct FileHash {
  XXH64_hash_t hash;
};


#define FileHash_STRLEN (2 * sizeof(struct FileHash))
#define FileHash_equal g_int64_equal
#define FileHash_hash g_int64_hash
#define FileHash_destroy(...)
#define FileHash_free g_free

inline char *FileHash_to_string (struct FileHash *hash, char *s) {
  if (s == NULL) {
    s = g_malloc(FileHash_STRLEN + 1);
  }
  buf2hex(s, &hash->hash, sizeof(hash->hash));
  s[FileHash_STRLEN] = '\0';
  return s;
}

inline int FileHash_init_copy (
    struct FileHash *hash, struct FileHash *orig) {
  memcpy(hash, orig, sizeof(struct FileHash));
  return 0;
}

inline struct FileHash *FileHash_new_copy (struct FileHash *hash) {
  return g_memdup(hash, sizeof(struct FileHash));
}

inline int FileHash_init_from_string (struct FileHash *hash, const char *s) {
  char *s_end;
  hash->hash = strtoull(s, &s_end, 16);
  if (s_end - s < FileHash_STRLEN) {
    return 1;
  }
  return 0;
}

inline int FileHash_init_from_buf (
    struct FileHash *hash, const void* buf, size_t size) {
  hash->hash = XXH64(buf, size, 0);
  return 0;
}

int FileHash_init_from_file (
    struct FileHash *hash, const char* path, GError **error);

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
