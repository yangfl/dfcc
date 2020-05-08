#ifndef DFCC_FILE_ETAG_H
#define DFCC_FILE_ETAG_H

#include <stdbool.h>
#include <sys/stat.h>

#include <glib.h>
#include <glib/gstdio.h>


/**
 * @ingroup File
 * @brief Contains the size and modified time of a file.
 */
struct FileETag {
  /// The size of a file.
  size_t size;
  /// The modified time of a file.
  time_t mtime;
};


/**
 * @memberof FileETag
 * @brief Tests if a stat buf is still valid for a FileETag.
 *
 * @param etag a FileETag
 * @param sb stat buf of a file
 * @return true if valid
 */
inline bool FileETag_isvalid_stat (
    const struct FileETag *etag, const GStatBuf *sb) {
  return etag->size == sb->st_size && etag->mtime >= sb->st_mtime;
}

/**
 * @memberof FileETag
 * @brief Tests if a file is still valid for a FileETag.
 *
 * @param etag a FileETag
 * @param path path to a file
 * @param[out] error a return location for a GError [optional]
 * @return true if valid
 */
bool FileETag_isvalid_path (
    const struct FileETag *etag, const char *path, GError **error);

/**
 * @memberof FileETag
 * @brief Frees associated resources of a FileETag.
 *
 * @param etag a FileETag
 */
inline void FileETag_destroy (struct FileETag *etag) {
}

/**
 * @memberof FileETag
 * @brief Frees a FileETag and associated resources.
 *
 * @param etag a FileETag
 */
void FileETag_free (void *etag);
/**
 * @memberof FileETag
 * @brief Initializes a FileETag with the stat buf `sb`.
 *
 * @param etag a FileETag
 * @param sb a GStatBuf
 * @return 0 if success, otherwize nonzero
 */
int FileETag_init_from_stat (struct FileETag *etag, GStatBuf *sb);
/**
 * @memberof FileETag
 * @brief Initializes a FileETag with the information of file `path`.
 *
 * @param etag a FileETag
 * @param path path to a file
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int FileETag_init_from_path (struct FileETag *etag, char *path, GError **error);


#endif /* DFCC_FILE_ETAG_H */
