#ifndef DFCC_FILE_STAT_H
#define DFCC_FILE_STAT_H

#include <stdbool.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "common/cdecls.h"

BEGIN_C_DECLS


/**
 * @ingroup File
 * @brief Contains the size and modified time of a file.
 */
struct FileStat {
  /// The size of a file.
  size_t size;
  /// The modified time of a file.
  time_t mtime;
};


/**
 * @memberof FileStat
 * @brief Tests if a stat buf is still valid for a FileStat.
 *
 * @param stat_ a FileStat
 * @param sb stat buf of a file
 * @return true if valid
 */
inline bool FileStat_isvalid_stat (
    const struct FileStat *stat_, const GStatBuf *sb) {
  return stat_->size == (unsigned) sb->st_size && stat_->mtime >= sb->st_mtime;
}

/**
 * @memberof FileStat
 * @brief Tests if a file is still valid for a FileStat.
 *
 * @param stat_ a FileStat
 * @param path path to a file
 * @param[out] error a return location for a GError [optional]
 * @return true if valid
 */
bool FileStat_isvalid_path (
    const struct FileStat *stat_, const char *path, GError **error);

/**
 * @memberof FileStat
 * @brief Frees associated resources of a FileStat.
 *
 * @param stat_ a FileStat
 */
inline void FileStat_destroy (struct FileStat *stat_) {
}

/**
 * @memberof FileStat
 * @brief Frees a FileStat and associated resources.
 *
 * @param stat_ a FileStat
 */
void FileStat_free (void *stat_);
/**
 * @memberof FileStat
 * @brief Initializes a FileStat with the stat buf `sb`.
 *
 * @param stat_ a FileStat
 * @param sb a GStatBuf
 * @return 0 if success, otherwize nonzero
 */
int FileStat_init_from_stat (struct FileStat *stat_, GStatBuf *sb);
/**
 * @memberof FileStat
 * @brief Initializes a FileStat with the information of file `path`.
 *
 * @param stat_ a FileStat
 * @param path path to a file
 * @param[out] error a return location for a GError [optional]
 * @return 0 if success, otherwize nonzero
 */
int FileStat_init_from_path (struct FileStat *stat_, char *path, GError **error);


END_C_DECLS

#endif /* DFCC_FILE_STAT_H */
