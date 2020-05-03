#include <errno.h>
#include <fcntl.h>

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
