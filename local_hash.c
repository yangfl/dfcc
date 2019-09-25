/* Store hash of local files */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <gmodule.h>

#include "macro.h"
#include "local_hash.h"


GHashTable *local_hash;


extern inline int FileInfo_init (struct FileInfo *this, off_t size, time_t mtime, XXH64_hash_t hash);
extern inline struct FileInfo *FileInfo_new (off_t size, time_t mtime, XXH64_hash_t hash);


int local_hash_init () {
  local_hash = g_hash_table_new_full(g_str_hash, g_str_equal, free, FileInfo_free);

  if unlikely (local_hash == NULL) {
    return 1;
  }

  return 0;
}


void local_hash_destory () {
  g_hash_table_destroy(local_hash);
}


struct FileInfo *local_hash_get (const char* path) {
  struct stat sb;
  if unlikely (stat(path, &sb) == -1) {
    handle_error("stat");
  }

  char *absolute_path = realpath(path, NULL);
  if unlikely (absolute_path == NULL) {
    handle_error("realpath");
  }

  struct FileInfo *cache_info = (struct FileInfo *) g_hash_table_lookup(local_hash, absolute_path);

  if (cache_info != NULL && cache_info->size == sb.st_size && cache_info->mtime == sb.st_mtime) {
    free(absolute_path);
    return cache_info;
  }

  int fd = open(absolute_path, O_RDONLY);
  if unlikely (fd == -1) {
    handle_error("open");
  }

  void *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if unlikely (addr == MAP_FAILED) {
    handle_error("mmap");
  }

  struct FileInfo *file_info = FileInfo_new(sb.st_size, sb.st_mtime, XXH64(addr, sb.st_size, 0));
  munmap(addr, sb.st_size);
  close(fd);

  g_hash_table_insert(local_hash, absolute_path, file_info);
  return file_info;
}
