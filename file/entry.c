#include <stdbool.h>

#include <glib.h>

#include "common/macro.h"
#include "stat.h"
#include "hash.h"
#include "entry.h"


extern inline void FileTag_destroy (struct FileTag *tag);
extern inline bool FileEntry_isvalid_weak (
    const struct FileEntry *entry, GError **error);
extern inline void FileEntry_destroy (struct FileEntry *entry);


void FileTag_free (void *tag) {
  FileTag_destroy(tag);
  g_free(tag);
}


int FileTag_init_with_hash (
    struct FileTag *tag, char *path, FileHash hash) {
  tag->path = path;
  tag->hash = hash;
  return 0;
}


int FileTag_init (
    struct FileTag *tag, char *path, GError **error) {
  tag->hash = FileHash_from_file(path, error);
  return_if_fail(tag->hash != 0) 1;

  tag->path = path;
  return 0;
}


int FileEntry_init_full (
    struct FileEntry *entry, char *path,
    GStatBuf *sb, FileHash hash) {
  int ret = FileTag_init_with_hash((struct FileTag *) entry, path, hash);
  should (ret == 0) otherwise return ret;

  return FileStat_init_from_stat(&entry->stat_, sb);
}


int FileEntry_init (
    struct FileEntry *entry, char *path,
    GStatBuf *sb, FileHash hash, GError **error) {
  int ret;
  if (hash == 0) {
    ret = FileTag_init((struct FileTag *) entry, path, error);
  } else {
    ret = FileTag_init_with_hash((struct FileTag *) entry, path, hash);
  }
  return_if_fail(ret == 0) ret;

  if (sb == NULL) {
    return FileStat_init_from_path(&entry->stat_, path, error);
  } else {
    return FileStat_init_from_stat(&entry->stat_, sb);
  }
}
