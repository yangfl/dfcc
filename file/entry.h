#ifndef DFCC_FILE_ENTRY_H
#define DFCC_FILE_ENTRY_H

#include <stdbool.h>

#include <glib.h>

#include "etag.h"
#include "hash.h"


/**
 * @ingroup File
 * @brief Contains the path to a file and its hash value.
 */
struct FileEntry {
  //! The path to a file.
  char *path;
  //! The hash value of the file.
  struct FileHash hash;
};


/**
 * @memberof FileEntry
 * @brief Frees associated resources of a FileEntry.
 *
 * @param entry a FileEntry
 */
inline void FileEntry_destroy (struct FileEntry *entry) {
  FileHash_destroy(&entry->hash);
}

/**
 * @memberof FileEntry
 * @brief Frees a FileEntry and associated resources.
 *
 * @param entry a FileEntry
 */
void FileEntry_free (void *entry);
/**
 * @memberof FileEntry
 * @brief Initializes a FileEntry with the path `path` and the hash `hash`.
 *
 * @param entry a FileEntry
 * @param path path to a file
 * @param hash hash of the file
 * @return 0 if success, otherwize nonzero
 */
int FileEntry_init_with_hash (
    struct FileEntry *entry, char *path, struct FileHash *hash);
/**
 * @memberof FileEntry
 * @brief Initializes a FileEntry with the file `path`.
 *
 * @param entry a FileEntry
 * @param path path to a file
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int FileEntry_init (
    struct FileEntry *entry, char *path, GError **error);


/**
 * @ingroup File
 * @extends FileEntry
 * @brief Contains the path, hash, size and modified time of a file.
 */
struct FileEntryE {
  struct FileEntry;
  /// ETag of the file.
  struct FileETag etag;
};


/**
 * @memberof FileEntryE
 * @brief Tests if FileEntryE.path and FileEntryE.etag is still matching.
 *
 * Does not rehash the file or check FileEntryE.hash.
 *
 * @param entrye a FileEntryE
 * @param[out] error a return location for a GError [optional]
 * @return true if matching
 */
inline bool FileEntryE_isvalid_weak (
    const struct FileEntryE *entrye, GError **error) {
  return FileETag_isvalid_path(&entrye->etag, entrye->path, error);
}

/**
 * @memberof FileEntryE
 * @brief Frees associated resources of a FileEntry.
 *
 * @param entrye a FileEntryE
 */
inline void FileEntryE_destroy (struct FileEntryE *entrye) {
  FileEntry_destroy((struct FileEntry *) entrye);
  FileETag_destroy(&entrye->etag);
}

/**
 * @memberof FileEntryE
 * @brief Initializes a FileEntryE with all the information of a file.
 *
 * @param entry a FileEntryE
 * @param path path to a file
 * @param sb stat buf of the file
 * @param hash hash of the file
 * @return 0 if success, otherwize nonzero
 */
int FileEntryE_init_full (
    struct FileEntryE *entrye, char *path, GStatBuf *sb, struct FileHash *hash);
/**
 * @memberof FileEntryE
 * @brief Initializes a FileEntryE with the information of a file.
 *
 * @param entry a FileEntryE
 * @param path path to a file
 * @param sb stat buf of the file [optional]
 * @param hash hash of the file [optional]
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int FileEntryE_init (
    struct FileEntryE *entrye, char *path,
    GStatBuf *sb, struct FileHash *hash, GError **error);

#endif /* DFCC_FILE_ENTRY_H */
