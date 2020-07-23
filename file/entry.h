#ifndef DFCC_FILE_ENTRY_H
#define DFCC_FILE_ENTRY_H

#include <stdbool.h>

#include <glib.h>

#include "common/cdecls.h"
#include "file/stat.h"
#include "file/hash.h"

BEGIN_C_DECLS


/**
 * @ingroup File
 * @brief Contains the path to a file and its hash value.
 */
struct FileTag {
  //! The path to a file.
  char *path;
  //! The hash value of the file.
  FileHash hash;
};


/**
 * @memberof FileTag
 * @brief Frees associated resources of a FileTag.
 *
 * @param tag a FileTag
 */
inline void FileTag_destroy (struct FileTag *tag) {
}

/**
 * @memberof FileTag
 * @brief Frees a FileTag and associated resources.
 *
 * @param tag a FileTag
 */
void FileTag_free (void *tag);
/**
 * @memberof FileTag
 * @brief Initializes a FileTag with the path `path` and the hash `hash`.
 *
 * @param tag a FileTag
 * @param path path to a file
 * @param hash hash of the file
 * @return 0 if success, otherwize nonzero
 */
int FileTag_init_with_hash (
    struct FileTag *tag, char *path, FileHash hash);
/**
 * @memberof FileTag
 * @brief Initializes a FileTag with the file `path`.
 *
 * @param tag a FileTag
 * @param path path to a file
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int FileTag_init (
    struct FileTag *tag, char *path, GError **error);


/**
 * @ingroup File
 * @extends FileTag
 * @brief Contains the path, hash, size and modified time of a file.
 */
struct FileEntry {
  struct FileTag ANON_MEMBER;
  /// Stat of the file.
  struct FileStat stat_;
};


/**
 * @memberof FileEntry
 * @brief Tests if FileEntry.path and FileEntry.stat_ is still matching.
 *
 * Does not rehash the file or check FileEntry.hash.
 *
 * @param entry a FileEntry
 * @param[out] error a return location for a GError [optional]
 * @return true if matching
 */
C_INLINE(bool FileEntry_isvalid_weak (
    const struct FileEntry *entry, GError **error), {
  return FileStat_isvalid_path(&entry->stat_, entry->path, error);
})

/**
 * @memberof FileEntry
 * @brief Frees associated resources of a FileTag.
 *
 * @param entry a FileEntry
 */
inline void FileEntry_destroy (struct FileEntry *entry) {
  FileTag_destroy((struct FileTag *) entry);
  FileStat_destroy(&entry->stat_);
}

/**
 * @memberof FileEntry
 * @brief Initializes a FileEntry with all the information of a file.
 *
 * @param entry a FileEntry
 * @param path path to a file
 * @param sb stat buf of the file
 * @param hash hash of the file
 * @return 0 if success, otherwize nonzero
 */
int FileEntry_init_full (
    struct FileEntry *entry, char *path, GStatBuf *sb, FileHash hash);
/**
 * @memberof FileEntry
 * @brief Initializes a FileEntry with the information of a file.
 *
 * @param entry a FileEntry
 * @param path path to a file
 * @param sb stat buf of the file [optional]
 * @param hash hash of the file [optional]
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int FileEntry_init (
    struct FileEntry *entry, char *path,
    GStatBuf *sb, FileHash hash, GError **error);


END_C_DECLS

#endif /* DFCC_FILE_ENTRY_H */
