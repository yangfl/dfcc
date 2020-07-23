#ifndef DFCC_FILE_CACHE_ENTRY_H
#define DFCC_FILE_CACHE_ENTRY_H

#include <stdbool.h>

#include <glib.h>

#include "common/cdecls.h"
#include "entry.h"

BEGIN_C_DECLS


/**
 * @ingroup File
 * @extends FileEntry
 * @brief Contains the information about a single cache file.
 *
 * @sa Cache
 */
struct CacheEntry {
  struct FileEntry ANON_MEMBER;
  /// Reference counter.
  gatomicrefcount arc;
  /// Whether the cache file is outdated.
  bool invalid;
};


/**
 * @memberof CacheEntry
 * @brief Frees associated resources of a CacheEntry.
 *
 * @param entry a CacheEntry
 */
inline void CacheEntry_destroy (struct CacheEntry *entry) {
  FileEntry_destroy((struct FileEntry *) entry);
}


/**
 * @memberof CacheEntry
 * @brief Releases a reference on entry.
 *
 * If the reference was the last one, it will free the allocated resources.
 *
 * @param entry a CacheEntry
 */
inline void CacheEntry_unref (struct CacheEntry *entry) {
  if (g_atomic_ref_count_dec(&entry->arc)) {
    CacheEntry_destroy(entry);
  }
}


/**
 * @memberof CacheEntry
 * @brief Acquires a reference on entry.
 *
 * @param entry a CacheEntry
 * @return entry
 */
inline struct CacheEntry *CacheEntry_ref (struct CacheEntry *entry) {
  g_atomic_ref_count_inc(&entry->arc);
  return entry;
}


/**
 * @memberof CacheEntry
 * @brief Initializes a CacheEntry with path, stat buffer and hash of a file.
 *
 * @param entry a CacheEntry
 * @param path path to a file
 * @param sb GStatBuf of a file
 * @param hash hash of a file
 * @return 0 if success, otherwize nonzero
 */
int CacheEntry_init (
  struct CacheEntry *entry, char *path, GStatBuf *sb, FileHash hash);


END_C_DECLS

#endif /* DFCC_FILE_CACHE_ENTRY_H */
