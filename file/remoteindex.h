#ifndef DFCC_FILE_REMOTE_INDEX_H
#define DFCC_FILE_REMOTE_INDEX_H

#include <gmodule.h>

#include <broadcast.h>

#include "entry.h"


//! @ingroup File
struct RemoteFileIndex {
  GHashTable *index;
  GRWLock index_rwlock;
  struct Broadcast sta;
};


//! @memberof RemoteFileIndex
bool RemoteFileIndex_add (
    struct RemoteFileIndex *index, struct FileEntry *entry, bool force);
//! @memberof RemoteFileIndex
struct FileEntry *RemoteFileIndex_get (
    struct RemoteFileIndex *index, const char* path);
//! @memberof RemoteFileIndex
struct FileEntry *RemoteFileIndex_try_get (
    struct RemoteFileIndex *index, const char* path);
/**
 * @memberof RemoteFileIndex
 * @brief Frees associated resources of a RemoteFileIndex.
 *
 * @param index a RemoteFileIndex
 */
void RemoteFileIndex_destroy (struct RemoteFileIndex *index);
//! @memberof RemoteFileIndex
int RemoteFileIndex_init (struct RemoteFileIndex *index);


#endif /* DFCC_FILE_REMOTE_INDEX_H */
