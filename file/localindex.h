#ifndef DFCC_FILE_LOCAL_INDEX_H
#define DFCC_FILE_LOCAL_INDEX_H

#include <gmodule.h>

#include "entry.h"


/**
 * @ingroup File
 * @struct LocalFileIndex
 * @extends GHashTable
 */

//! @memberof LocalFileIndex
struct FileEntryE *LocalFileIndex_get (
    GHashTable *index, const char* path, GError **error);

/**
 * @fn void LocalFileIndex_free (GHashTable *index)
 * @memberof LocalFileIndex
 * @brief Frees a LocalFileIndex and associated resources.
 *
 * @param index a LocalFileIndex
 */
#define LocalFileIndex_free g_hash_table_destroy
/**
 * @memberof LocalFileIndex
 * @brief Create a new LocalFileIndex.
 *
 * @return a new LocalFileIndex
 */
#define LocalFileIndex_new() g_hash_table_new_full(g_str_hash, g_str_equal, NULL, FileTag_free)

#endif /* DFCC_FILE_LOCAL_INDEX_H */
