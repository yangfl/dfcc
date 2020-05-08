#include <libsoup/soup.h>

#include "./version.h"


static void Server_Debug_format_HTTP_content (
    GString *buf, SoupMessageHeaders *header, SoupMessageBody *body) {
  SoupMessageHeadersIter iter;
  const char *name;
  const char *value;
  for (soup_message_headers_iter_init(&iter, header);
       soup_message_headers_iter_next(&iter, &name, &value);
      ) {
    g_string_append_printf(buf, "> %s: %s\n", name, value);
  }
  if (body->length) {
    g_string_append(buf, "\n");
    SoupBuffer *data = soup_message_body_flatten(body);
    g_string_append_len(buf, data->data, data->length);
    soup_buffer_free(data);
  }
}


static void Server_Debug_format_request (GString *buf, SoupMessage *msg,
                                         const char *path) {
  g_string_append_printf(buf, "> %s %s HTTP/1.%d\n", msg->method, path,
                  soup_message_get_http_version(msg));
  Server_Debug_format_HTTP_content(buf, msg->request_headers, msg->request_body);
}


static void Server_Debug_format_response (GString *buf, SoupMessage *msg) {
  g_string_append_printf(buf, "< HTTP/1.%d %u %s\n",
                         soup_message_get_http_version(msg), msg->status_code,
                         soup_status_get_phrase(msg->status_code));
  Server_Debug_format_HTTP_content(buf, msg->response_headers,
                                   msg->response_body);
}


void Server_Debug_request (SoupMessage *msg, const char *path) {
  GString *request = g_string_new("\n");
  Server_Debug_format_request(request, msg, path);
  g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, request->str);
  g_string_free(request, TRUE);
}


void Server_Debug_request_response (SoupMessage *msg, const char *path) {
  GString *request = g_string_new("\n");

  Server_Debug_format_request(request, msg, path);
  g_string_append_c(request, '\n');
  Server_Debug_format_response(request, msg);

  g_log(DFCC_NAME, G_LOG_LEVEL_DEBUG, request->str);
  g_string_free(request, TRUE);
}


void Server_Debug_listening (SoupServer *server) {
  GSList *uris = soup_server_get_uris(server);
  for (GSList *u = uris; u; u = u->next) {
    char *str = soup_uri_to_string(u->data, FALSE);
    g_log(DFCC_NAME, G_LOG_LEVEL_INFO, "Listening on %s", str);
    g_free(str);
    soup_uri_free(u->data);
  }
  g_slist_free(uris);
}
