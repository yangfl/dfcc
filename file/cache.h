#ifndef DFCC_FILE_CACHE_H
#define DFCC_FILE_CACHE_H

#include <stdbool.h>

#include <glib.h>
#include <gmodule.h>

#include <broadcast.h>

#include "entry.h"


inline bool FileEntry_is_cache (const struct FileEntry *entry) {
  return !g_path_is_absolute(entry->path);
}


struct CacheEntry;


struct Cache {
  // hash -> FileEntryE
  // relative path to cache_dir
  GHashTable *index;
  GRWLock index_rwlock;

  struct Broadcast sta;

  char *cache_dir;
  unsigned int cache_dir_len;

  bool no_verify_cache;
};


// no null terminator
#define CACHE_SUBDIR_LENGTH 5
#define CACHE_FILENAME_LENGTH FileHash_STRLEN
#define CACHE_RELPATH_LENGTH (CACHE_SUBDIR_LENGTH + CACHE_FILENAME_LENGTH + 1)


char *Cache_realpath (const struct Cache *cache, char *cache_relpath);
bool Cache_verify (
    struct Cache *cache, struct CacheEntry *entrye, GError **error);
struct FileEntryE *Cache_get (
    struct Cache *cache, struct FileHash *hash, GError **error);
struct FileEntryE *Cache_try_get (
    struct Cache *cache, struct FileHash *hash, GError **error);
struct FileEntryE *Cache_index_buf (
    struct Cache *cache, const char *buf, size_t size, GError **error);
struct FileEntryE *Cache_index_path (
    struct Cache *cache, const char *path, bool *added, GError **error);
void Cache_destroy (struct Cache *cache);
int Cache_init (struct Cache *cache, char *cache_dir, bool no_verify_cache);

#endif /* DFCC_FILE_CACHE_H */
