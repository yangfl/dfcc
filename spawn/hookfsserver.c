#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "common/simplestring.h"
#include "common/wrapper/file.h"
#include "hookfs/limit.h"
#include "hookfs/serializer.h"
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


static bool G_GNUC_UNUSED HookFsServer__remove_sockfile (const char *path) {
  if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
    return true;
  }

  GStatBuf sb;
  should (g_stat(path, &sb) == 0) otherwise return false;

  if (S_ISSOCK(sb.st_mode)) {
    g_unlink(path);
    return true;
  }
  return false;
}


void HookFsServer_destroy (struct HookFsServer *server) {
  g_object_unref(server->service);
  g_free(server->path);
}


//! @memberof HookFsServer
struct HookFsServerConnection {
  GSocketConnection *connection;
  struct HookFsServer *server;
  HookFsID id;
  char message[Hookfs_MAX_PACKET_LEN];
};


static void HookFsServerConnection_message_receive_cb (
    GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GInputStream *istream = G_INPUT_STREAM(source_object);
  struct HookFsServerConnection *conn = user_data;

  GError *error = NULL;
  int count = g_input_stream_read_finish(istream, res, &error);
  should (count >= 0) otherwise {
    g_error("Error when receiving message");
    if (error != NULL) {
      g_error("%s", error->message);
      g_clear_error(&error);
    }
  }

  if unlikely (count <= 0) {
    goto close;
  }

  g_message("Message was: \"%s\"", conn->message);
  int tokens[Hookfs_MAX_TOKENS];
  int tokens_len = 0;

  for (int i = 0;;) {
    SimpleString__size_t next_token_len =
      *((SimpleString__size_t *) (conn->message + i));
    if (next_token_len == 0) {
      break;
    }
    tokens[tokens_len] = i;
    tokens_len++;
    i += next_token_len;
  }

  if unlikely (tokens_len == 0) {
    goto next;
  }

  const char *func_name = ((struct SimpleString *) (conn->message))->str;
  if (func_name[0] == '-') {
    should (tokens_len > 1) otherwise {
      // error
      goto close;
    }
    const char *func_arg1 = tokens_len > 1 ?
      ((struct SimpleString *) (conn->message + tokens[1]))->str : NULL;
    if (strcmp(func_name, "-id") == 0) {
      char *func_arg1_end;
      conn->id = strtoull(func_arg1, &func_arg1_end, 10);
      should (*func_arg1_end == '\0') otherwise {
        // error
        goto close;
      }
    }
  }

next:
  // schedule next read
  g_input_stream_read_async(
    istream, conn->message, sizeof(conn->message),
    G_PRIORITY_DEFAULT, NULL, HookFsServerConnection_message_receive_cb, conn);
  return;

close:
  // Connection closed
  g_print("Connection closed.\n");
  g_object_unref(conn->connection);
  g_free(conn);
  return;
}


gboolean HookFsServer_incoming_callback (
    GSocketService *service, GSocketConnection *connection,
    GObject *source_object, struct HookFsServer *server) {
  g_print("Received Connection from client!\n");
  GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

  struct HookFsServerConnection *conn = g_new(struct HookFsServerConnection, 1);
  conn->connection = g_object_ref(connection);
  conn->server = server;
  conn->id = 0;

  g_input_stream_read_async(
    istream, conn->message, sizeof(conn->message),
    G_PRIORITY_DEFAULT, NULL, HookFsServerConnection_message_receive_cb, conn);
  return FALSE;
}


int HookFsServer_init (
    struct HookFsServer *server, const char *path,
    HookFsServerFileTranslator translator, GError **error) {
  int ret = 0;

  server->path = g_strdup_printf(path, getpid());
  server->service = g_socket_service_new();

  do_once {
    GSocketAddress *address = g_unix_socket_address_new_with_type(
      server->path, -1, G_UNIX_SOCKET_ADDRESS_ABSTRACT);
    bool err = g_socket_listener_add_address(
        G_SOCKET_LISTENER(server->service), address,
        G_SOCKET_TYPE_SEQPACKET, G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, error);
    g_object_unref(address);
    if (!err) {
      ret = 1;
      break;
    }

    g_signal_connect(server->service, "incoming",
                     G_CALLBACK(HookFsServer_incoming_callback), server);
    g_print ("Waiting for client!\n");
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    server->translator = translator;
    return 0;
  }

  g_object_unref(server->service);
  g_free(server->path);
  return ret;
}
