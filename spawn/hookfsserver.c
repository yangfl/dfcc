#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "common/macro.h"
#include "common/simplestring.h"
#include "common/wrapper/file.h"
#include "hookfs/limit.h"
#include "hookfs/serializer.h"
#include "log.h"
#include "hookedprocessgroup.h"
#include "hookfsserver.h"


static bool G_GNUC_UNUSED HookFsServer__remove_sockfile (const char *socket_path) {
  if (!g_file_test(socket_path, G_FILE_TEST_EXISTS)) {
    return true;
  }

  GStatBuf sb;
  should (g_stat(socket_path, &sb) == 0) otherwise return false;

  if (S_ISSOCK(sb.st_mode)) {
    g_unlink(socket_path);
    return true;
  }
  return false;
}


void HookFsServer_destroy (struct HookFsServer *server) {
  g_object_unref(server->service);
  g_free(server->socket_path);
}


//! @memberof HookFsServer
struct HookFsServerConnection {
  GSocketConnection *connection;
  struct HookFsServer *server;
  struct HookedProcess *p;
  char message[Hookfs_MAX_PACKET_LEN];
};


static void HookFsServerConnection_message_receive_cb (
    GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GInputStream *istream = G_INPUT_STREAM(source_object);
  struct HookFsServerConnection *conn = user_data;

  GError *error = NULL;
  int count = g_input_stream_read_finish(istream, res, &error);
  should (count >= 0) otherwise {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
          "Error when receiving message: %s", error->message);
    g_error_free(error);
  }

  if unlikely (count <= 0) {
    goto close;
  }

  struct SimpleString *tokens[Hookfs_MAX_TOKENS];
  int tokens_len = 0;
  for (int i = 0; i < count;) {
    tokens[tokens_len] = (struct SimpleString *) (conn->message + i);
    if (tokens[tokens_len]->len == 0) {
      break;
    }
    i += SIMPLE_STRING_SIZEOF(tokens[tokens_len]);
    tokens_len++;
  }

  if unlikely (tokens_len == 0) {
    goto next;
  }

  const char *func_name = tokens[0]->str;
  if (func_name[0] == '-') {
    should (tokens_len > 1) otherwise {
      g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
            "Too few arguements for '%s'", func_name);
      goto close;
    }
    if (strcmp(func_name, "-id") == 0) {
      char *func_arg1_end;
      HookedProcessGroupID hgid = strtoull(tokens[1]->str, &func_arg1_end, 16);
      should (*func_arg1_end == '\0') otherwise {
        g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
              "Cannot parse HookFs group id: %s", tokens[1]->str);
        goto close;
      }
      GPid pid = *((uint32_t *) tokens[2]->str);
      conn->p = conn->server->resolver(conn->server, hgid, pid);
      should (conn->p != NULL) otherwise {
        g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
              "Unknown hooked process %x:%d", hgid, pid);
        goto close;
      }
      g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_DEBUG,
            "Get HookFs connection from %x:%d", hgid, pid);
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
  if unlikely (conn->p == NULL) {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
          "HookFs connection closed from unknown process");
  } else {
    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_DEBUG,
          "HookFs connection %x:%d closed", conn->p->group->hgid, conn->p->pid);
  }
  g_object_unref(conn->connection);
  g_free(conn);
  return;
}


gboolean HookFsServer_incoming_callback (
    GSocketService *service, GSocketConnection *connection,
    GObject *source_object, struct HookFsServer *server) {
  GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

  struct HookFsServerConnection *conn = g_new(struct HookFsServerConnection, 1);
  conn->connection = g_object_ref(connection);
  conn->server = server;
  conn->p = NULL;

  g_input_stream_read_async(
    istream, conn->message, sizeof(conn->message),
    G_PRIORITY_DEFAULT, NULL, HookFsServerConnection_message_receive_cb, conn);
  return FALSE;
}


int HookFsServer_init (
    struct HookFsServer *server, const char *socket_path,
    HookFsServerFileTranslator translator, HookFsServerProcessResolver resolver,
    GError **error) {
  int ret = 0;

  server->socket_path = g_strdup_printf(socket_path, getpid());
  server->service = g_socket_service_new();

  do_once {
    GSocketAddress *address = g_unix_socket_address_new_with_type(
      server->socket_path, -1, G_UNIX_SOCKET_ADDRESS_ABSTRACT);
    bool err = g_socket_listener_add_address(
        G_SOCKET_LISTENER(server->service), address,
        G_SOCKET_TYPE_SEQPACKET, G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, error);
    g_object_unref(address);
    if (!err) {
      ret = 1;
      break;
    }

    server->translator = translator;
    server->resolver = resolver;

    g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_DEBUG,
          "HookFs server listen at '@%s'", server->socket_path);
    g_signal_connect(server->service, "incoming",
                     G_CALLBACK(HookFsServer_incoming_callback), server);
    return 0;
  }

  g_object_unref(server->service);
  g_free(server->socket_path);
  return ret;
}
