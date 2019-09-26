#include <gmodule.h>

#include <broadcast.h>

#include "entry.h"
#include "remoteindex.h"


bool RemoteFileIndex_add (
    struct RemoteFileIndex *index, struct FileEntry *entry, bool force) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&index->index_rwlock);
  gboolean cached = g_hash_table_contains(index->index, entry->path);
  g_rw_lock_reader_locker_free(locker);

  if (cached && !force) {
    return false;
  }

  g_rw_lock_writer_lock(&index->index_rwlock);
  g_hash_table_insert(index->index, entry->path, entry);
  g_rw_lock_writer_unlock(&index->index_rwlock);

  if (!cached) {
    Broadcast_send(&index->sta, entry->path, entry);
  }

  return true;
}


struct FileEntry *RemoteFileIndex_try_get (
    struct RemoteFileIndex *index, const char* path) {
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&index->index_rwlock);
  struct FileEntry *entry = g_hash_table_lookup(index->index, path);
  g_rw_lock_reader_locker_free(locker);
  return entry;
}


struct FileEntry *RemoteFileIndex_get (
    struct RemoteFileIndex *index, const char* path) {
  struct FileEntry *entry = RemoteFileIndex_try_get(index, path);
  if (entry != NULL) {
    return entry;
  }

  return Broadcast_listen(
    &index->sta, entry->path, (QueryFunc) RemoteFileIndex_try_get, index);
}


void RemoteFileIndex_destroy (struct RemoteFileIndex *index) {
  g_hash_table_destroy(index->index);
  g_rw_lock_clear(&index->index_rwlock);
  Broadcast_destroy(&index->sta);
}


int RemoteFileIndex_init (struct RemoteFileIndex *index) {
  int ret = Broadcast_init(
    &index->sta, g_str_hash, g_str_equal, (DupFunc) g_strdup, g_free);
  should (ret == 0) otherwise return ret;
  index->index =
    g_hash_table_new_full(g_str_hash, g_str_equal, NULL, FileEntry_free);
  g_rw_lock_init(&index->index_rwlock);
  return 0;
}
