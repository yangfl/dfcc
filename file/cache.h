#ifndef DFCC_FILE_CACHE_H
#define DFCC_FILE_CACHE_H

#include <stdbool.h>

#include <glib.h>
#include <gmodule.h>

#include <broadcast.h>

#include "entry.h"


/**
 * @relates Cache
 * @brief Tests if a FileEntry represents a cache file.
 *
 * @param entry a FileEntry
 * @return true if `entry` represents a cache file
 */
inline bool FileEntry_is_cache (const struct FileEntry *entry) {
  return !g_path_is_absolute(entry->path);
}


struct CacheEntry;


/**
 * @ingroup File
 * @brief Contains the information of the cache storage.
 */
struct Cache {
  // hash -> FileEntryE
  // relative path to cache_dir
  /**
   * @brief Hash table from file path to CacheEntry
   *
   * Relative paths are relative to Config.cache_dir.
   *
   * Absolute paths means local non-cache files.
   */
  GHashTable *index;
  /// Lock for Cache.index.
  GRWLock index_rwlock;

  struct Broadcast sta;

  /// The base directory to store cache files.
  char *cache_dir;
  /// Length of Cache.cache_dir.
  unsigned int cache_dir_len;

  /// Whether to check cached files against its claimed hash.
  bool no_verify_cache;
};


/**
 * @ingroup File
 * @brief The length of the cache subdir name.
 */
#define Cache_SUBDIR_LENGTH 5
/**
 * @ingroup File
 * @brief The length of a cache file name.
 */
#define Cache_FILENAME_LENGTH FileHash_STRLEN
/**
 * @ingroup File
 * @brief The length of a cache file path relative to Config.cache_dir.
 */
#define Cache_RELPATH_LENGTH (Cache_SUBDIR_LENGTH + Cache_FILENAME_LENGTH + 1)


/**
 * @memberof Cache
 * @brief Get the absolute path to the cache file.
 *
 * `cache_relpath` is meant to be CacheEntry.path, which could actually be a
 * absolute path, in that case `cache_relpath` is simply duplicated and
 * returned.
 *
 * @param cache a Cache
 * @param cache_relpath relative path from CacheEntry
 * @return the absolute path [transfer-full]
 */
char *Cache_realpath (const struct Cache *cache, char *cache_relpath);
bool Cache_verify (
    struct Cache *cache, struct CacheEntry *entrye, GError **error);
/**
 * @memberof Cache
 * @brief Looks up a hash in a Cache.
 *
 * @param cache a Cache
 * @param hash the FileHash to look up
 * @param[out] error a return location for a GError [optional]
 * @return the associated FileEntryE, or NULL if error happened [transfer-none]
 */
struct FileEntryE *Cache_get (
    struct Cache *cache, struct FileHash *hash, GError **error);
struct FileEntryE *Cache_try_get (
    struct Cache *cache, struct FileHash *hash, GError **error);
/**
 * @memberof Cache
 * @brief Stores a piece of data into Cache.
 *
 * @param cache a Cache
 * @param buf the data buf
 * @param size length of `buf`
 * @param[out] error a return location for a GError [optional]
 * @return the associated FileEntryE, or NULL if error happened [transfer-none]
 */
struct FileEntryE *Cache_index_buf (
    struct Cache *cache, const char *buf, size_t size, GError **error);
/**
 * @memberof Cache
 * @brief Adds a local file into Cache database, but not copying the content of
 *        file.
 *
 * @param cache a Cache
 * @param path path to a file
 * @param[out] added whether the file `path` has just added into database
 * @param[out] error a return location for a GError [optional]
 * @return the associated FileEntryE, or NULL if error happened [transfer-none]
 */
struct FileEntryE *Cache_index_path (
    struct Cache *cache, const char *path, bool *added, GError **error);
/**
 * @memberof Cache
 * @brief Frees associated resources of a Cache.
 *
 * @param cache a Cache
 */
void Cache_destroy (struct Cache *cache);
/**
 * @memberof Cache
 * @brief Initializes a Cache with SoupServer instance and Config.
 *
 * @param cache a Cache
 * @param cache_dir the base directory to store cache files
 * @param no_verify_cache whether to check cached files against its claimed hash
 * @return 0 if success, otherwize nonzero
 */
int Cache_init (struct Cache *cache, char *cache_dir, bool no_verify_cache);

#endif /* DFCC_FILE_CACHE_H */
