#include <gmodule.h>

#include <macro.h>

#include "common.h"
#include "entry.h"
#include "localindex.h"


struct FileEntryE *LocalFileIndex_get (
    GHashTable *index, const char* path, GError **error) {
  GStatBuf sb;
  should (g_stat_e(path, &sb, error)) otherwise return NULL;

  char *absolute_path = g_canonicalize_filename(path, NULL);
  struct FileEntryE *entrye = g_hash_table_lookup(index, absolute_path);
  bool cached = entrye != NULL;

  if (cached) {
    free(absolute_path);
    if (FileETag_isvalid_stat(&entrye->etag, &sb)) {
      return entrye;
    } else {
      FileHash_destroy(&entrye->hash);
      FileETag_destroy(&entrye->etag);
      absolute_path = entrye->path;
    }
  } else {
    entrye = g_malloc(sizeof(struct FileEntryE));
  }

  int ret = FileEntryE_init(entrye, absolute_path, &sb, NULL, error);
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
