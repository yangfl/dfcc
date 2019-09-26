#include <stdbool.h>
#include <string.h>

#include <glib.h>

#include "../file/common.h"
#include "hookfsserver.h"


char *HookFsServer_parse (const char *buf, unsigned int size, char **mode) {
  unsigned int mode_len = strlen(buf);
  *mode = g_memdup(buf, mode_len + 1);
  return g_memdup(buf + mode_len + 1, size - mode_len - 1);
}


char *HookFsServer_get (GString *buf, int fd, char **mode, GError **error) {
  while (1) {
    char read_buf[1024];
    int read_len = read_e(fd, read_buf, sizeof(read_buf), error);
    if (read_len <= 0) {
      return NULL;
    }
    g_string_append_len(buf, read_buf, read_len);
    if (read_len < sizeof(read_buf)) {
      break;
    }
  }
  if (buf->str[buf->len - 1] != '\n') {
    return NULL;
  }
  char *missing_path = HookFsServer_parse(buf->str, buf->len, mode);
  g_string_truncate(buf, 0);
  return missing_path;
}


int HookFsServer__send (int fd, const char *virtual_path, const char *real_path, GError **error) {
  unsigned int virtual_path_len = strlen(virtual_path);
  unsigned int real_path_len = strlen(real_path);
  char buf[virtual_path_len + real_path_len + 2];
  memcpy(buf, virtual_path, virtual_path_len + 1);
  memcpy(buf + virtual_path_len + 1, real_path, real_path_len + 1);
  buf[sizeof(buf) - 1] = '\n';
  return write_e(fd, buf, sizeof(buf), error) < 0;
}
