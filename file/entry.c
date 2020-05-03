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
    struct FileEntry *entry, char *path, FileHash hash) {
  entry->path = path;
  entry->hash = hash;
  return 0;
}


int FileEntry_init (
    struct FileEntry *entry, char *path, GError **error) {
  entry->hash = FileHash_from_file(path, error);
  return_if_fail(entry->hash != 0) 1;

  entry->path = path;
  return 0;
}


int FileEntryE_init_full (
    struct FileEntryE *entrye, char *path,
    GStatBuf *sb, FileHash hash) {
  int ret = FileEntry_init_with_hash((struct FileEntry *) entrye, path, hash);
  should (ret == 0) otherwise return ret;

  return FileETag_init_from_stat(&entrye->etag, sb);
}


int FileEntryE_init (
    struct FileEntryE *entrye, char *path,
    GStatBuf *sb, FileHash hash, GError **error) {
  int ret;
  if (hash == 0) {
    ret = FileEntry_init((struct FileEntry *) entrye, path, error);
  } else {
    ret = FileEntry_init_with_hash((struct FileEntry *) entrye, path, hash);
  }
  return_if_fail(ret == 0) ret;

  if (sb == NULL) {
    return FileETag_init_from_path(&entrye->etag, path, error);
  } else {
    return FileETag_init_from_stat(&entrye->etag, sb);
  }
}
