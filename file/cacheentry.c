#include <stdbool.h>

#include <glib.h>

#include "entry.h"
#include "cacheentry.h"


extern inline void CacheEntry_destroy (struct CacheEntry *entry);
extern inline void CacheEntry_unref (struct CacheEntry *entry);
extern inline struct CacheEntry *CacheEntry_ref (struct CacheEntry *entry);


int CacheEntry_init (
    struct CacheEntry *entry, char *path, GStatBuf *sb, FileHash hash) {
  g_atomic_ref_count_init(&entry->arc);
  CacheEntry_ref(entry);
  entry->invalid = false;
  return FileEntry_init_full((struct FileEntry *) entry, path, sb, hash);
}


