#include <gmodule.h>

#include "entry.h"
#include "remoteindex.h"


bool RemoteFileIndex_add (
    struct RemoteFileIndex *index, struct FileTag *tag, bool force) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&index->rwlock);
  gboolean cached = g_hash_table_contains(index->table, tag->path);
  g_rw_lock_reader_locker_free(locker);

  if (cached && !force) {
    return false;
  }

  g_rw_lock_writer_lock(&index->rwlock);
  g_hash_table_insert(index->table, tag->path, tag);
  g_rw_lock_writer_unlock(&index->rwlock);

  return true;
}


struct FileTag *RemoteFileIndex_get (
    struct RemoteFileIndex *index, const char* path) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&index->rwlock);
  struct FileTag *tag = g_hash_table_lookup(index->table, path);
  g_rw_lock_reader_locker_free(locker);
  return tag;
}


void RemoteFileIndex_destroy (struct RemoteFileIndex *index) {
  g_hash_table_destroy(index->table);
  g_rw_lock_clear(&index->rwlock);
}


void RemoteFileIndex_free (void *index) {
  RemoteFileIndex_destroy(index);
  g_free(index);
}


int RemoteFileIndex_init (struct RemoteFileIndex *index) {
  index->table =
    g_hash_table_new_full(g_str_hash, g_str_equal, NULL, FileTag_free);
  g_rw_lock_init(&index->rwlock);
  return 0;
}
