#include <gmodule.h>

#include "common/macro.h"
#include "common/wrapper/file.h"
#include "entry.h"
#include "localindex.h"


struct FileEntry *LocalFileIndex_get (
    GHashTable *index, const char* path, GError **error) {
  GStatBuf sb;
  return_if_fail(g_stat_e(path, &sb, error) == 0) NULL;

  char *absolute_path = g_canonicalize_filename(path, NULL);
  struct FileEntry *entry = g_hash_table_lookup(index, absolute_path);
  bool cached = entry != NULL;

  if (cached) {
    free(absolute_path);
    if (FileStat_isvalid_stat(&entry->stat_, &sb)) {
      return entry;
    } else {
      FileStat_destroy(&entry->stat_);
      absolute_path = entry->path;
    }
  } else {
    entry = g_malloc(sizeof(struct FileEntry));
  }

  int ret = FileEntry_init(entry, absolute_path, &sb, 0, error);
  should (ret == 0) otherwise {
    if (cached) {
      g_hash_table_remove(index, absolute_path);
    }
    g_free(absolute_path);
    g_free(entry);
    return NULL;
  }

  g_hash_table_insert(index, entry->path, entry);
  return entry;
}


void LocalFileIndex_destroy (struct LocalFileIndex *index) {
  g_hash_table_destroy(index->table);
}


int LocalFileIndex_init (struct LocalFileIndex *index) {
  index->table = g_hash_table_new_full(
    g_str_hash, g_str_equal, NULL, FileStat_free);
  return 0;
}
