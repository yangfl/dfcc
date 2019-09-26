#ifndef DFCC_FILE_ENTRY_H
#define DFCC_FILE_ENTRY_H

#include <stdbool.h>

#include <glib.h>

#include "etag.h"
#include "hash.h"


struct FileEntry {
  char *path;
  struct FileHash hash;
};


inline void FileEntry_destroy (struct FileEntry *entry) {
  FileHash_destroy(&entry->hash);
}

void FileEntry_free (void *entry);
int FileEntry_init_with_hash (
    struct FileEntry *entry, char *path, struct FileHash *hash);
int FileEntry_init (
    struct FileEntry *entry, char *path, GError **error);


struct FileEntryE {
  struct FileEntry;
  struct FileETag etag;
};


inline bool FileEntryE_isvalid_weak (
    const struct FileEntryE *entrye, GError **error) {
  return FileETag_isvalid_path(&entrye->etag, entrye->path, error);
}

inline void FileEntryE_destroy (struct FileEntryE *entrye) {
  FileEntry_destroy((struct FileEntry *) entrye);
  FileETag_destroy(&entrye->etag);
}

int FileEntryE_init_full (
    struct FileEntryE *entrye, char *path, GStatBuf *sb, struct FileHash *hash);
int FileEntryE_init (
    struct FileEntryE *entrye, char *path,
    GStatBuf *sb, struct FileHash *hash, GError **error);

#endif /* DFCC_FILE_ENTRY_H */
