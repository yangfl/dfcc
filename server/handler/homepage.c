#include "common/macro.h"
#include "./version.h"
#include "middleware.h"
#include "homepage.h"


const char SOUP_HANDLER_PATH(Server_handle_homepage)[] = "";


void Server_handle_homepage (
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
    SoupClientContext *context, gpointer user_data) {
  SOUP_HANDLER_MIDDLEWARE(Server_handle_homepage, false, false);
  if (msg->method == SOUP_METHOD_GET || msg->method == SOUP_METHOD_HEAD) {
    static const char homepage[] = "\
<html>\r\n\
<body>\r\n\
Hi, " DFCC_NAME " " DFCC_VERSION "\r\n\
</body>\r\n\
</html>\r\n\
";
    soup_message_set_response(msg, "text/html",
                              SOUP_MEMORY_STATIC, homepage, strlen(homepage));
    soup_message_set_status(msg, SOUP_STATUS_OK);
  } else if unlikely (msg->method == SOUP_METHOD_POST) {
    soup_message_set_status(msg, SOUP_STATUS_METHOD_NOT_ALLOWED);
  } else {
    soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
}
