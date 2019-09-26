#ifndef DFCC_FILE_REMOTE_INDEX_H
#define DFCC_FILE_REMOTE_INDEX_H

#include <gmodule.h>

#include <broadcast.h>

#include "entry.h"


struct RemoteFileIndex {
  GHashTable *index;
  GRWLock index_rwlock;
  struct Broadcast sta;
};


bool RemoteFileIndex_add (
    struct RemoteFileIndex *index, struct FileEntry *entry, bool force);
struct FileEntry *RemoteFileIndex_get (
    struct RemoteFileIndex *index, const char* path);
struct FileEntry *RemoteFileIndex_try_get (
    struct RemoteFileIndex *index, const char* path);
void RemoteFileIndex_destroy (struct RemoteFileIndex *index);
int RemoteFileIndex_init (struct RemoteFileIndex *index);


#endif /* DFCC_FILE_REMOTE_INDEX_H */
