#ifndef DFCC_FILE_LOCAL_INDEX_H
#define DFCC_FILE_LOCAL_INDEX_H

#include <gmodule.h>


//! @ingroup File
struct LocalFileIndex {
  GHashTable *table;
};


//! @memberof LocalFileIndex
struct FileEntryE *LocalFileIndex_get (
    GHashTable *index, const char* path, GError **error);
/**
 * @memberof LocalFileIndex
 * @brief Frees associated resources of a LocalFileIndex.
 *
 * @param index a LocalFileIndex
 */
void LocalFileIndex_destroy (struct LocalFileIndex *index);
/**
 * @memberof LocalFileIndex
 * @brief Initializes a LocalFileIndex.
 *
 * @param index a LocalFileIndex
 * @return 0 if success, otherwize nonzero
 */
int LocalFileIndex_init (struct LocalFileIndex *index);


#endif /* DFCC_FILE_LOCAL_INDEX_H */
