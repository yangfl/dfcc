#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include <glib.h>

#include <hexstring.h>
#include <macro.h>
#include <wrapper/mappedfile.h>

#include "../version.h"
#include "hash.h"


extern inline char *FileHash_to_buf (FileHash hash, char *s);
extern inline char *FileHash_to_string (FileHash hash, char *s);
extern inline FileHash FileHash_from_string (const char *s);
extern inline FileHash FileHash_from_buf (const void* buf, size_t size);


FileHash FileHash_from_file (const char* path, GError **error) {
  struct MappedFile m;
  should (MappedFile_init(&m, path, error) == 0) otherwise {
    return 1;
  }
  FileHash hash = FileHash_from_buf(m.content, m.length);
  MappedFile_destroy(&m);
  return hash;
}
