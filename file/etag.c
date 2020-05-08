#include <stdbool.h>

#include <glib.h>

#include <macro.h>
#include <wrapper/file.h>

#include "etag.h"


extern inline bool FileETag_isvalid_stat (
    const struct FileETag *etag, const GStatBuf *sb);
extern inline void FileETag_destroy (struct FileETag *etag);


bool FileETag_isvalid_path (
    const struct FileETag *etag, const char *path, GError **error) {
  GStatBuf sb;
  should (g_stat_e(path, &sb, error)) otherwise return false;
  return FileETag_isvalid_stat(etag, &sb);
}


void FileETag_free (void *etag) {
  FileETag_destroy(etag);
  g_free(etag);
}


int FileETag_init_from_stat (struct FileETag *etag, GStatBuf *sb) {
  etag->size = sb->st_size;
  etag->mtime = sb->st_mtime;
  return 0;
}


int FileETag_init_from_path (struct FileETag *etag, char *path, GError **error) {
  GStatBuf sb;
  int ret = g_stat_e(path, &sb, error);
  should (ret == 0) otherwise return ret;
  return FileETag_init_from_stat(etag, &sb);
}
