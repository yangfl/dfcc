#include <gmodule.h>

#include <macro.h>
#include <wrapper/file.h>

#include "entry.h"
#include "localindex.h"


struct FileEntryE *LocalFileIndex_get (
    GHashTable *index, const char* path, GError **error) {
  GStatBuf sb;
  return_if_fail(g_stat_e(path, &sb, error) == 0) NULL;

  char *absolute_path = g_canonicalize_filename(path, NULL);
  struct FileEntryE *entrye = g_hash_table_lookup(index, absolute_path);
  bool cached = entrye != NULL;

  if (cached) {
    free(absolute_path);
    if (FileETag_isvalid_stat(&entrye->etag, &sb)) {
      return entrye;
    } else {
      FileETag_destroy(&entrye->etag);
      absolute_path = entrye->path;
    }
  } else {
    entrye = g_malloc(sizeof(struct FileEntryE));
  }

  int ret = FileEntryE_init(entrye, absolute_path, &sb, 0, error);
  should (ret == 0) otherwise {
    if (cached) {
      g_hash_table_remove(index, absolute_path);
    }
    g_free(absolute_path);
    g_free(entrye);
    return NULL;
  }

  g_hash_table_insert(index, entrye->path, entrye);
  return entrye;
}


void LocalFileIndex_destroy (struct LocalFileIndex *index) {
  g_hash_table_destroy(index->table);
}


int LocalFileIndex_init (struct LocalFileIndex *index) {
  index->table = g_hash_table_new_full(
    g_str_hash, g_str_equal, NULL, FileETag_free);
  return 0;
}
