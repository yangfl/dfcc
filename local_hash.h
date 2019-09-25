#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <xxhash.h>

#include "macro.h"


struct FileInfo {
  off_t size;
  time_t mtime;
  XXH64_hash_t hash;
};


inline int FileInfo_init (struct FileInfo *this, off_t size, time_t mtime, XXH64_hash_t hash) {
  this->size = size;
  this->mtime = mtime;
  this->hash = hash;
  return 0;
}


inline struct FileInfo *FileInfo_new (off_t size, time_t mtime, XXH64_hash_t hash) {
  struct FileInfo *this = (struct FileInfo *) malloc(sizeof(struct FileInfo));
  if unlikely (this == NULL) {
    //handle_error("malloc");
  }
  FileInfo_init(this, size, mtime, hash);
  return this;
}


#define FileInfo_free free


int local_hash_init ();
void local_hash_destory ();
struct FileInfo *local_hash_get (const char* path);
#define local_hash_add local_hash_get
