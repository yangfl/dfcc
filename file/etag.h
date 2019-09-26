#ifndef DFCC_FILE_ETAG_H
#define DFCC_FILE_ETAG_H

#include <stdbool.h>
#include <sys/stat.h>

#include <glib.h>
#include <glib/gstdio.h>


struct FileETag {
  size_t size;
  time_t mtime;
};


inline bool FileETag_isvalid_stat (
    const struct FileETag *etag, const GStatBuf *sb) {
  return etag->size == sb->st_size && etag->mtime >= sb->st_mtime;
}

bool FileETag_isvalid_path (
    const struct FileETag *etag, const char *path, GError **error);

inline void FileETag_destroy (struct FileETag *etag) {
}

void FileETag_free (struct FileETag *etag);
int FileETag_init_from_stat (struct FileETag *etag, GStatBuf *sb);
int FileETag_init_from_path (struct FileETag *etag, char *path, GError **error);


#endif /* DFCC_FILE_ETAG_H */
