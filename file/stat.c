#include <stdbool.h>

#include <glib.h>

#include "common/macro.h"
#include "common/wrapper/file.h"
#include "stat.h"


extern inline bool FileStat_isvalid_stat (
    const struct FileStat *stat_, const GStatBuf *sb);
extern inline void FileStat_destroy (struct FileStat *stat_);


bool FileStat_isvalid_path (
    const struct FileStat *stat_, const char *path, GError **error) {
  GStatBuf sb;
  should (g_stat_e(path, &sb, error)) otherwise return false;
  return FileStat_isvalid_stat(stat_, &sb);
}


void FileStat_free (void *stat_) {
  FileStat_destroy(stat_);
  g_free(stat_);
}


int FileStat_init_from_stat (struct FileStat *stat_, GStatBuf *sb) {
  stat_->size = sb->st_size;
  stat_->mtime = sb->st_mtime;
  return 0;
}


int FileStat_init_from_path (struct FileStat *stat_, char *path, GError **error) {
  GStatBuf sb;
  int ret = g_stat_e(path, &sb, error);
  should (ret == 0) otherwise return ret;
  return FileStat_init_from_stat(stat_, &sb);
}
