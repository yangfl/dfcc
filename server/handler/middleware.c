/**
 * @addtogroup ServerHandler
 * @{
 */

#include <stdbool.h>

#include <libsoup/soup.h>

#include "common/macro.h"
#include "common/morestring.h"
#include "common/wrapper/soup.h"
#include "../debug.h"
#include "../protocol.h"
#include "middleware.h"


/**
 * @brief Checks if `path` is a proper path under the toplevel for the handler.
 *
 * @param server_ctx the ServerContext
 * @param path the path component of the request
 * @param prefix_len the length of the toplevel path for the handler
 * @return `true` if proper
 */
static inline bool Server_is_path_proper (
    struct ServerContext *server_ctx,
    const char *path, unsigned int prefix_len) {
  path += server_ctx->config->base_path_len + prefix_len;
  if (path[0] == '/') {
    path++;
  }
  if unlikely (path[0] != '\0') {
    return false;
  }
  return true;
}


struct Session *Server_handle_middleware (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, struct ServerContext *server_ctx,
    bool proper_required, int prefix_len) {
  // check for proper path
  if (proper_required) {
    should (Server_is_path_proper(server_ctx, path, prefix_len)) otherwise {
      soup_message_set_status(msg, SOUP_STATUS_NOT_FOUND);
      return NULL;
    }
  }

  SessionID sid = SessionID_INVAILD;

  // extract sid from cookies
  gchar **cookies = g_strsplit(
    soup_message_headers_get_one(msg->request_headers, "Cookie"), "; ", 0);
  for (int i = 0; cookies[i] != NULL; i++) {
    if (strscmp(cookies[i], DFCC_COOKIES_SID) == 0) {
      const char *s_sid = cookies[i] + strlen(DFCC_COOKIES_SID);
      char *s_sid_end;
      sid = strtoull(s_sid, &s_sid_end, 16);
      if (s_sid == s_sid_end) {
        sid = SessionID_INVAILD;
      }
      break;
    }
  }
  g_strfreev(cookies);

  return SessionID_vaild(sid) ?
    SessionTable_get(&server_ctx->session_table, sid) : NULL;
}


/**@}*/
