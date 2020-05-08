#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib.h>

#include <macro.h>
#include <wrapper/file.h>

#include "mappedfile.h"


void MappedFile_destroy (struct MappedFile *m) {
  if (m->mapped != NULL) {
    g_mapped_file_unref(m->mapped);
  } else {
    g_free(m->content);
  }
}


int MappedFile_init_from_fd (struct MappedFile *m, int fd, GError **error) {
  int ret = 0;
  GError *error_ = NULL;

  // mmap
  do_once {
    m->mapped = g_mapped_file_new_from_fd(fd, FALSE, &error_);
    should (m->mapped != NULL) otherwise {
      char path[PATH_MAX];
      should (readfd(fd, path, sizeof(path)) >= 0) otherwise {
        strcpy(path, "[not found]");
      }
      g_log("wrapper", G_LOG_LEVEL_WARNING,
            "Cannot mmap fd %d (%s): %s", fd, path, error_->message);
      g_error_free(error_);
      m->mapped = NULL;
      break;
    }
    m->content = g_mapped_file_get_contents(m->mapped);
    m->length = g_mapped_file_get_length(m->mapped);
    return 0;
  }

  // fallback (fd)
  off_t orig_offset = lseek(fd, 0, SEEK_CUR);

  off_t fsize = lseek(fd, 0, SEEK_END);
  should (fsize >= 0 && lseek(fd, 0, SEEK_SET) >= 0) otherwise {
    int saved_errno = errno;
    char path[PATH_MAX];
    should (readfd(fd, path, sizeof(path)) >= 0) otherwise {
      strcpy(path, "[not found]");
    }
    g_set_error(error, G_FILE_ERROR, saved_errno,
                "Cannot lseek fd %d (%s): %s",
                fd, path, g_strerror(saved_errno));
    return 1;
  }
  m->length = fsize;
  m->content = g_malloc(m->length);

  do_once {
    ssize_t size_read = read(fd, m->content, m->length);
    should (m->length == size_read) otherwise {
      int saved_errno = errno;
      char path[PATH_MAX];
      should (readfd(fd, path, sizeof(path)) >= 0) otherwise {
        strcpy(path, "[not found]");
      }
      g_set_error(error, G_FILE_ERROR, saved_errno,
                  "Cannot fread fd %d (%s), expect %zu, got %zd: %s",
                  fd, path, m->length, size_read, g_strerror(saved_errno));
      ret = 1;
      break;
    }

    goto succ;
  }

  g_free(m->content);

succ:
  lseek(fd, orig_offset, SEEK_SET);
  return ret;
}


int MappedFile_init (struct MappedFile *m, const char *path, GError **error) {
  int ret = 0;
  GError *error_ = NULL;

  // mmap
  do_once {
    m->mapped = g_mapped_file_new(path, FALSE, &error_);
    should (m->mapped != NULL) otherwise {
      g_log("wrapper", G_LOG_LEVEL_WARNING,
            "Cannot mmap file %s: %s", path, error_->message);
      g_error_free(error_);
      m->mapped = NULL;
      break;
    }
    m->content = g_mapped_file_get_contents(m->mapped);
    m->length = g_mapped_file_get_length(m->mapped);
    return 0;
  }

  // fallback (stdc)
  FILE *f = g_fopen_e(path, "r", error);
  should (f != NULL) otherwise {
    return 1;
  }

  do_once {
    fseek(f, 0, SEEK_END);
    m->length = ftell(f);
    fseek(f, 0, SEEK_SET);
    m->content = g_malloc(m->length);

    do_once {
      size_t size_read = fread(m->content, m->length, 1, f);
      should (m->length == size_read) otherwise {
        int saved_errno = errno;
        g_set_error(error, G_FILE_ERROR, saved_errno,
                    "Cannot fread file %s, expect %zu, got %zu: %s",
                    path, m->length, size_read, g_strerror(saved_errno));
        ret = 1;
        break;
      }

      goto succ;
    }

    g_free(m->content);
  }

succ:
  fclose(f);
  return ret;
}
