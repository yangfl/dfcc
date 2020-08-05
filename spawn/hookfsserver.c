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


void HookFsServer_destroy (struct HookFsServer *server) {
  g_object_unref(server->service);
  g_free(server->socket_path);
}


//! @memberof HookFsServer
struct HookFsServerConnection {
  GSocketConnection *connection;
  struct HookFsServer *server;
  struct HookedProcess *p;

  GPtrArray *tokens;

  GVariantBuilder builder;
  enum {
    CONNECTION_GOT_TYPE,
    CONNECTION_GOT_LENGTH,
    CONNECTION_GOT_CONTENT
  } phase;
  bool build_array;

  uint8_t type;
  void *buf;
  uint64_t num;
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

  switch (conn->phase) {
    case CONNECTION_GOT_TYPE:
      switch (conn->type) {
        case MESSAGE_END:
          if (conn->build_array) {
            // array end
            g_ptr_array_add(conn->tokens, g_variant_builder_end(&conn->builder));
            conn->build_array = false;
          } else {
            // message end
            const char *func_name = g_variant_get_string(conn->tokens->pdata[0], NULL);
            if (func_name[0] == '-') {
              // special control message
              should (conn->tokens->len > 1) otherwise {
                g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
                      "Too few arguements for '%s'", func_name);
                goto close;
              }
              const char *func_arg1 = g_variant_get_string(conn->tokens->pdata[1], NULL);

              if (strcmp(func_name, "-id") == 0) {
                char *func_arg1_end;
                HookedProcessGroupID hgid = strtoull(func_arg1, &func_arg1_end, 16);
                should (*func_arg1_end == '\0') otherwise {
                  g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
                        "Cannot parse HookFs group id: %s", func_arg1);
                  goto close;
                }
                GPid pid = g_variant_get_uint64(conn->tokens->pdata[2]);
                conn->p = conn->server->resolver(conn->server, hgid, pid);
                should (conn->p != NULL) otherwise {
                  g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_WARNING,
                        "Unknown hooked process %x:%d", hgid, pid);
                  goto close;
                }
                g_log(DFCC_SPAWN_NAME, G_LOG_LEVEL_DEBUG,
                      "Get HookFs connection from %x:%d", hgid, pid);
              }
            } else {
            }
          }
          goto read_type;
        case MESSAGE_ARRAY:
          // read array content
          g_variant_builder_init(&conn->builder, G_VARIANT_TYPE_STRING_ARRAY);
          conn->build_array = true;
          conn->phase = CONNECTION_GOT_TYPE;
          goto read_num;
        case MESSAGE_NUMERICAL:
          // read number
          conn->phase = CONNECTION_GOT_CONTENT;
          goto read_num;
        case MESSAGE_STRING:
          // read length of string
          conn->phase = CONNECTION_GOT_LENGTH;
          goto read_num;
      }
    case CONNECTION_GOT_LENGTH:
      // read string
      conn->buf = g_malloc(conn->num);
      conn->phase = CONNECTION_GOT_CONTENT;
      goto read_string;
    case CONNECTION_GOT_CONTENT: {
      GVariant *value;
      switch (conn->type) {
        case MESSAGE_NUMERICAL:
          // store number
          value = g_variant_new_uint64(conn->num);
          break;
        case MESSAGE_STRING:
          // store sting
          value = g_variant_new_take_string(conn->buf);
          conn->buf = NULL;
          break;
        default:
          break;
      }
      if (conn->build_array) {
        g_variant_builder_add_value(&conn->builder, value);
      } else {
        g_variant_ref_sink(value);
        g_ptr_array_add(conn->tokens, value);
      }
      conn->phase = CONNECTION_GOT_TYPE;
      goto read_type;
    }
  }

read_type:
  g_input_stream_read_async(
    istream, &conn->type, sizeof(conn->type),
    G_PRIORITY_DEFAULT, NULL, HookFsServerConnection_message_receive_cb, conn);
  return;

read_num:
  g_input_stream_read_async(
    istream, &conn->num, sizeof(conn->num),
    G_PRIORITY_DEFAULT, NULL, HookFsServerConnection_message_receive_cb, conn);
  return;

read_string:
  g_input_stream_read_async(
    istream, conn->buf, conn->num,
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

  if unlikely (conn->build_array) {
    g_variant_unref(g_variant_builder_end(&conn->builder));
  }
  if unlikely (conn->buf != NULL) {
    g_free(conn->buf);
  }
  g_ptr_array_free(conn->tokens, TRUE);
  g_object_unref(conn->connection);
  g_free(conn);
  return;
}


gboolean HookFsServer_incoming_callback (
    GSocketService *service, GSocketConnection *connection,
    GObject *source_object, struct HookFsServer *server) {
  GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

  struct HookFsServerConnection *conn = g_new0(struct HookFsServerConnection, 1);
  conn->connection = g_object_ref(connection);
  conn->server = server;
  conn->tokens = g_ptr_array_new_with_free_func((GDestroyNotify) g_variant_unref);

  g_input_stream_read_async(
    istream, &conn->type, sizeof(conn->type),
    G_PRIORITY_DEFAULT, NULL, HookFsServerConnection_message_receive_cb, conn);
  return FALSE;
}


int HookFsServer_init (
    struct HookFsServer *server, const char *socket_path,
    HookFsServerProcessResolver resolver, GError **error) {
  int ret = 0;

  server->socket_path = g_strdup_printf(socket_path, getpid());
  server->service = g_socket_service_new();

  do_once {
    GSocketAddress *address = g_unix_socket_address_new_with_type(
      server->socket_path, -1, G_UNIX_SOCKET_ADDRESS_ABSTRACT);
    bool err = g_socket_listener_add_address(
        G_SOCKET_LISTENER(server->service), address,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL, NULL, error);
    g_object_unref(address);
    if (!err) {
      ret = 1;
      break;
    }

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
