#include <stdbool.h>
#include <fcntl.h>

#include <gmodule.h>

#include <broadcast.h>
#include <hexstring.h>
#include <macro.h>

#include "../version.h"
#include "common.h"
#include "entry.h"
#include "cache.h"


struct CacheEntry {
  struct FileEntryE;
  gatomicrefcount arc;
  bool invalid;
};


void CacheEntry_destroy (struct CacheEntry *entrye) {
  FileEntryE_destroy((struct FileEntryE *) entrye);
}


int CacheEntry_init (
    struct CacheEntry *entrye, char *path,
    GStatBuf *sb, struct FileHash *hash) {
  g_atomic_ref_count_init(&entrye->arc);
  entrye->invalid = false;
  return FileEntryE_init_full((struct FileEntryE *) entrye, path, sb, hash);
}


extern inline bool FileEntry_is_cache (const struct FileEntry *entry);


static int Cache__construct_subdir (
    struct FileHash *hash, char *cache_subdir) {
  uint8_t *s_hash = (uint8_t *) &hash->hash;
  cache_subdir[0] = chr2hex(s_hash[0] >> 4);
  cache_subdir[1] = '/';
  cache_subdir[2] = chr2hex(s_hash[0] & 0xF);
  cache_subdir[3] = '/';
  cache_subdir[4] = chr2hex(s_hash[1] >> 4);
  return 0;
}


static int Cache__construct_relpath (
    struct FileHash *hash, char *cache_relpath) {
  Cache__construct_subdir(hash, cache_relpath);
  cache_relpath[CACHE_SUBDIR_LENGTH] = '/';
  FileHash_to_string(hash, cache_relpath + CACHE_SUBDIR_LENGTH + 1);
  return 0;
}


static char *Cache_realpath_force (
    const struct Cache *cache, char *cache_relpath) {
  char *cache_abspath =
    g_malloc(cache->cache_dir_len + 1 + CACHE_RELPATH_LENGTH + 1);
  strcpy(cache_abspath, cache->cache_dir);
  cache_abspath[cache->cache_dir_len] = '/';
  strcpy(&cache_abspath[cache->cache_dir_len + 1], cache_relpath);
  return cache_abspath;
}


char *Cache_realpath (const struct Cache *cache, char *cache_relpath) {
  if unlikely (g_path_is_absolute(cache_relpath)) {
    return g_strdup(cache_relpath);
  } else {
    return Cache_realpath_force(cache, cache_relpath);
  }
}


static inline char *Cache_construct_abspath (
    const struct Cache *cache, struct FileHash *hash) {
  char cache_relpath[CACHE_RELPATH_LENGTH + 1];
  Cache__construct_relpath(hash, cache_relpath);
  return Cache_realpath_force(cache, cache_relpath);
}


bool Cache_verify (
    struct Cache *cache, struct CacheEntry *entrye, GError **error) {
  bool entry_valid = false;

  if (!entrye->invalid) {
    bool entry_is_cache = FileEntry_is_cache((struct FileEntry *) entrye);

    char *path;
    // convert to absolute path if necessary
    if (entry_is_cache) {
      path = Cache_realpath(cache, entrye->path);
    } else {
      path = entrye->path;
    }

    do_once {
      GStatBuf sb;
      should (g_stat_e(path, &sb, error) == 0) otherwise break;
      entry_valid = FileETag_isvalid_stat(&entrye->etag, &sb);

      if (!entry_valid) {
        // invalid stamp, try to rescue
        // size must match
        if (sb.st_size == entrye->etag.size) {
          struct FileHash current_hash;
          should (FileHash_init_from_file(
            &current_hash, path, error) == 0) otherwise break;
          // hash must match
          if (FileHash_equal(&current_hash, &entrye->hash)) {
            // so we can rescue
            entrye->etag.mtime = sb.st_mtime;
            entry_valid = true;
          }
        }
      }
    }

    if (!entry_valid) {
      // invalid entry, delete
      g_rw_lock_writer_lock(&cache->index_rwlock);
      g_hash_table_remove(cache->index, &entrye->hash);
      g_rw_lock_writer_unlock(&cache->index_rwlock);

      if (entry_is_cache) {
        // invalid cache file, delete
        g_remove(path);
      }
      entrye->invalid = true;
    }

    if (entry_is_cache) {
      g_free(path);
    }
  }

  if (g_atomic_ref_count_dec(&entrye->arc) && entrye->invalid) {
    CacheEntry_destroy(entrye);
    g_free(entrye);
  }

  return entry_valid;
}


static struct FileEntryE *Cache_index (
    struct Cache *cache, char *path, GStatBuf *sb, struct FileHash *hash) {
  struct CacheEntry *entrye = g_malloc(sizeof(struct CacheEntry));
  CacheEntry_init(entrye, path, sb, hash);

  GRWLockWriterLocker *locker =
    g_rw_lock_writer_locker_new(&cache->index_rwlock);
  g_hash_table_insert(cache->index, &entrye->hash, entrye);
  g_rw_lock_writer_locker_free(locker);

  return (struct FileEntryE *) entrye;
}


static struct FileEntryE *Cache_index_cache (
    struct Cache *cache, struct FileHash *hash, GError **error) {
  char cache_relpath[CACHE_RELPATH_LENGTH + 1];
  Cache__construct_relpath(hash, cache_relpath);
  char *cache_abspath = Cache_realpath_force(cache, cache_relpath);

  do_once {
    // file should exist
    if (g_access(cache_abspath, F_OK) != 0) {
      break;
    }

    do_once {
      // file should match its hash
      if (!cache->no_verify_cache) {
        struct FileHash file_hash;
        should (FileHash_init_from_file(
          &file_hash, cache_abspath, error) == 0) otherwise break;
        if (!FileHash_equal(hash, &file_hash)) {
          break;
        }
      }

      // ok, add to db
      GStatBuf sb;
      should (g_stat_e(cache_abspath, &sb, error) == 0) otherwise break;
      free(cache_abspath);
      return Cache_index(cache, g_memdup(cache_relpath, sizeof(cache_relpath)), &sb, hash);
    }

    g_remove(cache_abspath);
  }

  free(cache_abspath);
  return NULL;
}


struct FileEntryE *Cache_try_get (
    struct Cache *cache, struct FileHash *hash, GError **error) {
  // find an existing one
  GRWLockReaderLocker *locker =
    g_rw_lock_reader_locker_new(&cache->index_rwlock);
  struct CacheEntry *entrye = g_hash_table_lookup(cache->index, hash);
  if (entrye != NULL) {
    g_atomic_ref_count_inc(&entrye->arc);
  }
  g_rw_lock_reader_locker_free(locker);

  if (entrye != NULL) {
    GError *error_ = NULL;
    if (Cache_verify(cache, entrye, &error_)) {
      return (struct FileEntryE *) entrye;
    }
    if (error_ != NULL) {
      g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
            "Error when verify cache entry: %s", error_->message);
      g_error_free(error_);
    }
  }

  return Cache_index_cache(cache, hash, error);
}


struct FileEntryE *Cache_index_buf (
    struct Cache *cache, const char *buf, size_t size, GError **error) {
  struct FileHash hash;
  FileHash_init_from_buf(&hash, buf, size);
  GError *error_ = NULL;
  struct FileEntryE *entrye = Cache_try_get(cache, &hash, error);
  should (error_ == NULL) otherwise {
    if (error != NULL) {
      *error = error_;
    }
    return NULL;
  }
  if (entrye != NULL) {
    return entrye;
  }

  // write to cache dir
  char cache_relpath[CACHE_RELPATH_LENGTH + 1];
  Cache__construct_relpath(&hash, cache_relpath);
  char *cache_abspath = Cache_realpath(cache, cache_relpath);

  do_once {
    // mkdir -p
    cache_abspath[cache->cache_dir_len + 1 + CACHE_SUBDIR_LENGTH] = '\0';
    should (g_mkdir_with_parents_e(
      cache_abspath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, error) == 0
    ) otherwise break;
    cache_abspath[cache->cache_dir_len + 1 + CACHE_SUBDIR_LENGTH] = '/';

    // write
    FILE *f = g_fopen_e(cache_abspath, "w", error);
    should (f != NULL) otherwise break;

    size_t wrote_len = fwrite_e(buf, size, 1, f, error);
    fclose(f);
    should (wrote_len == size) otherwise break;

    GStatBuf sb;
    should (g_stat_e(cache_abspath, &sb, error) == 0) otherwise break;
    free(cache_abspath);
    return Cache_index(
      cache, g_memdup(cache_relpath, sizeof(cache_relpath)), &sb, &hash);
  }

  g_remove(cache_abspath);
  free(cache_abspath);
  return NULL;
}


struct FileEntryE *Cache_index_path (
    struct Cache *cache, const char *path, bool *added, GError **error) {
  bool added_ = false;
  struct FileEntryE *entrye = NULL;

  char *cache_abspath = NULL;
  if (!g_path_is_absolute(path)) {
    cache_abspath = g_canonicalize_filename(path, NULL);
    path = cache_abspath;
  }

  do_once {
    if unlikely (g_str_has_prefix(path, cache->cache_dir)) {
      // we do not add a cache file
      break;
    }

    // get the hash of the file
    struct FileHash hash;
    should (FileHash_init_from_file(&hash, path, error) == 0) otherwise break;

    // does the hash exist in our db?
    GError *error_ = NULL;
    struct FileEntryE *existing_entry = Cache_try_get(cache, &hash, &error_);
    should (error_ == NULL) otherwise {
      if (error != NULL) {
        *error = error_;
      }
      break;
    }
    if (existing_entry != NULL &&
        !FileEntry_is_cache((struct FileEntry *) existing_entry)) {
      // we already knew that file
      break;
    }

    // add to db
    GStatBuf sb;
    should (g_stat_e(path, &sb, error) == 0) otherwise break;
    entrye = Cache_index(
      cache, cache_abspath == NULL ? g_strdup(path) : cache_abspath,
      &sb, &hash);
    should (entrye != NULL) otherwise break;
    added_ = true;
  }

  if (!added_) {
    free(cache_abspath);
  }
  if (added != NULL) {
    *added = added_;
  }
  return entrye;
}


void Cache_destroy (struct Cache *cache) {
  g_hash_table_destroy(cache->index);
  g_rw_lock_clear(&cache->index_rwlock);
  Broadcast_destroy(&cache->sta);
  g_free(cache->cache_dir);
}


int Cache_init (struct Cache *cache, char *cache_dir, bool no_verify_cache) {
  int ret = Broadcast_init(
    &cache->sta, FileHash_hash, FileHash_equal,
    (DupFunc) FileHash_new_copy, FileHash_free);
  should (ret == 0) otherwise return ret;
  cache->index =
    g_hash_table_new_full(FileHash_hash, FileHash_equal, NULL, NULL);
  g_rw_lock_init(&cache->index_rwlock);
  cache->cache_dir = cache_dir;
  cache->cache_dir_len = strlen(cache_dir);
  cache->no_verify_cache = no_verify_cache;
  return 0;
}
