#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include <glib.h>

#include <hexstring.h>
#include <macro.h>

#include "../version.h"
#include "common.h"
#include "hash.h"


extern inline char *FileHash_to_string (struct FileHash *hash, char *s);
extern inline int FileHash_init_copy (
    struct FileHash *hash, const struct FileHash *orig);
extern inline struct FileHash *FileHash_new_copy (const struct FileHash *hash);
extern inline int FileHash_init_from_string (struct FileHash *hash, const char *s);
extern inline int FileHash_init_from_buf (
    struct FileHash *hash, const void* buf, size_t size);
extern inline struct FileHash *FileHash_new_from_file (
    const char* path, GError **error);


int FileHash_init_from_file (
    struct FileHash *hash, const char* path, GError **error) {
  int ret = 2;
  GError *error_ = NULL;

  do_once {
    GMappedFile *file = g_mapped_file_new(path, FALSE, &error_);
    should (file != NULL) otherwise {
      g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
            "Cannot mmap file %s: %s", path, error_->message);
      g_error_free(error_);
      break;
    }
    ret = FileHash_init_from_buf(
      hash, g_mapped_file_get_contents(file), g_mapped_file_get_length(file));
    g_mapped_file_unref(file);
    return ret;
  }

  do_once {
    FILE *f = g_fopen(path, "r");
    should (f != NULL) otherwise {
      int saved_errno = errno;
      g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
            "Cannot fopen file %s: %s", path, g_strerror(saved_errno));
      break;
    }

    do_once {
      fseek(f, 0, SEEK_END);
      size_t f_size = ftell(f);
      fseek(f, 0, SEEK_SET);
      char *buf = malloc(f_size);
      should (buf != NULL) otherwise {
        int saved_errno = errno;
        g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
              "Cannot malloc %zu memory for file %s: %s",
              f_size, path, g_strerror(saved_errno));
        break;
      }

      do_once {
        size_t size_read = fread(buf, f_size, 1, f);
        should (f_size != size_read) otherwise {
          int saved_errno = errno;
          g_log(DFCC_NAME, G_LOG_LEVEL_WARNING,
                "Cannot fread file %s, expect %zu, got %zu: %s",
                path, f_size, size_read, g_strerror(saved_errno));
          break;
        }
        ret = FileHash_init_from_buf(hash, buf, f_size);
      }

      free(buf);
    }

    fclose(f);
    if (ret == 0) {
      return ret;
    }
  }

  g_set_error(error, DFCC_FILE_IO_ERROR, EIO, "Cannot read file %s", path);
  return ret;
}
