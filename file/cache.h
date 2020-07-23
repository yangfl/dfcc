#ifndef DFCC_FILE_CACHE_H
#define DFCC_FILE_CACHE_H

#include <stdbool.h>

#include <glib.h>
#include <gmodule.h>

#include "common/cdecls.h"
#include "cacheentry.h"

BEGIN_C_DECLS


/**
 * @relates Cache
 * @brief Tests if a FileTag represents a cache file.
 *
 * @param tag a FileTag
 * @return true if `entry` represents a cache file
 */
inline bool FileTag_is_cache (const struct FileTag *tag) {
  return !g_path_is_absolute(tag->path);
}


/**
 * @ingroup File
 * @brief Contains the information of the cache storage.
 */
struct Cache {
  /**
   * @brief Hash table from file path to CacheEntry
   *
   * Relative paths are relative to Config.cache_dir.
   *
   * Absolute paths means local non-cache files.
   */
  GHashTable *index;
  /// Lock for Cache.index.
  GRWLock rwlock;

  /// The base directory to store cache files.
  const char *cache_dir;
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
    struct Cache *cache, struct CacheEntry *entry, GError **error);
/**
 * @memberof Cache
 * @brief Looks up a hash in memory only.
 *
 * @param cache a Cache
 * @param hash the FileHash to look up
 * @param[out] error a return location for a GError [optional]
 * @return the associated CacheEntry, or NULL if error happened [transfer-none]
 */
struct CacheEntry *Cache_try_get (struct Cache *cache, FileHash hash);
/**
 * @memberof Cache
 * @brief Looks up a hash in a Cache.
 *
 * @param cache a Cache
 * @param hash the FileHash to look up
 * @param[out] error a return location for a GError [optional]
 * @return the associated CacheEntry, or NULL if error happened [transfer-none]
 */
struct CacheEntry *Cache_get (
    struct Cache *cache, FileHash hash, GError **error);
/**
 * @memberof Cache
 * @brief Stores a piece of data into Cache.
 *
 * @param cache a Cache
 * @param buf the data buf
 * @param size length of `buf`
 * @param[out] error a return location for a GError [optional]
 * @return the associated CacheEntry, or NULL if error happened [transfer-none]
 */
struct CacheEntry *Cache_index_buf (
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
 * @return the associated CacheEntry, or NULL if error happened [transfer-none]
 */
struct CacheEntry *Cache_index_path (
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
int Cache_init (struct Cache *cache, const char *cache_dir, bool no_verify_cache);


END_C_DECLS

#endif /* DFCC_FILE_CACHE_H */
