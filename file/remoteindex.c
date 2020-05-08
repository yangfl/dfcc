#include <gmodule.h>

#include "common/broadcast.h"
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

  if (!cached) {
    Broadcast_send(&index->sta, tag->path, tag);
  }

  return true;
}


struct FileTag *RemoteFileIndex_try_get (
    struct RemoteFileIndex *index, const char* path) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&index->rwlock);
  struct FileTag *tag = g_hash_table_lookup(index->table, path);
  g_rw_lock_reader_locker_free(locker);
  return tag;
}


struct FileTag *RemoteFileIndex_get (
    struct RemoteFileIndex *index, const char* path) {
  struct FileTag *tag = RemoteFileIndex_try_get(index, path);
  if (tag != NULL) {
    return tag;
  }

  return Broadcast_listen(
    &index->sta, tag->path, (QueryFunc) RemoteFileIndex_try_get, index);
}


void RemoteFileIndex_destroy (struct RemoteFileIndex *index) {
  g_hash_table_destroy(index->table);
  g_rw_lock_clear(&index->rwlock);
  Broadcast_destroy(&index->sta);
}


void RemoteFileIndex_free (void *index) {
  RemoteFileIndex_destroy(index);
  g_free(index);
}


int RemoteFileIndex_init (struct RemoteFileIndex *index) {
  int ret = Broadcast_init(
    &index->sta, g_str_hash, g_str_equal, (DupFunc) g_strdup, g_free);
  should (ret == 0) otherwise return ret;
  index->table =
    g_hash_table_new_full(g_str_hash, g_str_equal, NULL, FileTag_free);
  g_rw_lock_init(&index->rwlock);
  return 0;
}
