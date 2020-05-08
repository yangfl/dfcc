#ifndef DFCC_FILE_REMOTE_INDEX_H
#define DFCC_FILE_REMOTE_INDEX_H

#include <gmodule.h>

#include "common/broadcast.h"
#include "file/entry.h"


//! @ingroup File
struct RemoteFileIndex {
  GHashTable *table;
  GRWLock rwlock;
  struct Broadcast sta;
};


//! @memberof RemoteFileIndex
bool RemoteFileIndex_add (
    struct RemoteFileIndex *index, struct FileTag *tag, bool force);
//! @memberof RemoteFileIndex
struct FileTag *RemoteFileIndex_get (
    struct RemoteFileIndex *index, const char* path);
//! @memberof RemoteFileIndex
struct FileTag *RemoteFileIndex_try_get (
    struct RemoteFileIndex *index, const char* path);
/**
 * @memberof RemoteFileIndex
 * @brief Frees associated resources of a RemoteFileIndex.
 *
 * @param index a RemoteFileIndex
 */
void RemoteFileIndex_destroy (struct RemoteFileIndex *index);
/**
 * @memberof RemoteFileIndex
 * @brief Frees a RemoteFileIndex and its associated resources.
 *
 * @param index a RemoteFileIndex
 */
void RemoteFileIndex_free (void *index);
/**
 * @memberof RemoteFileIndex
 * @brief Initializes a RemoteFileIndex.
 *
 * @param index a RemoteFileIndex
 * @return 0 if success, otherwize nonzero
 */
int RemoteFileIndex_init (struct RemoteFileIndex *index);


#endif /* DFCC_FILE_REMOTE_INDEX_H */
