#include <stdbool.h>

#include <glib.h>

#include <macro.h>

#include "etag.h"
#include "hash.h"
#include "entry.h"


extern inline void FileEntry_destroy (struct FileEntry *entry);
extern inline bool FileEntryE_isvalid_weak (
    const struct FileEntryE *entrye, GError **error);
extern inline void FileEntryE_destroy (struct FileEntryE *entrye);


void FileEntry_free (void *entry) {
  FileEntry_destroy((struct FileEntry *) entry);
  g_free(entry);
}


int FileEntry_init_with_hash (
    struct FileEntry *entry, char *path, struct FileHash *hash) {
  int ret = FileHash_init_copy(&entry->hash, hash);
  should (ret == 0) otherwise return ret;

  entry->path = path;
  return 0;
}


int FileEntry_init (
    struct FileEntry *entry, char *path, GError **error) {
  int ret = FileHash_init_from_file(&entry->hash, path, error);
  should (ret == 0) otherwise return ret;

  entry->path = path;
  return 0;
}


int FileEntryE_init_full (
    struct FileEntryE *entrye, char *path,
    GStatBuf *sb, struct FileHash *hash) {
  int ret = FileEntry_init_with_hash((struct FileEntry *) entrye, path, hash);
  should (ret == 0) otherwise return ret;

  return FileETag_init_from_stat(&entrye->etag, sb);
}


int FileEntryE_init (
    struct FileEntryE *entrye, char *path,
    GStatBuf *sb, struct FileHash *hash, GError **error) {
  int ret;
  if (hash == NULL) {
    ret = FileEntry_init((struct FileEntry *) entrye, path, error);
  } else {
    ret = FileEntry_init_with_hash((struct FileEntry *) entrye, path, hash);
  }
  should (ret == 0) otherwise return ret;

  if (sb == NULL) {
    return FileETag_init_from_path(&entrye->etag, path, error);
  } else {
    return FileETag_init_from_stat(&entrye->etag, sb);
  }
}
