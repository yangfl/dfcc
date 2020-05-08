#ifndef DFCC_SERVER_HANDLER_COMMON_H
#define DFCC_SERVER_HANDLER_COMMON_H
/**
 * @addtogroup ServerHandler
 * @{
 */

#include "server/context.h"
#include "server/session.h"


#define SOUP_HANDLER(handler) \
  void handler ( \
    SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, \
    SoupClientContext *context, gpointer user_data)

#define SOUP_HANDLER_PATH(handler) handler ## __path

#define SOUP_HANDLER_PROTOTYPE(handler) \
  SOUP_HANDLER(handler); \
  extern const char SOUP_HANDLER_PATH(handler)[]

#define ADD_HANDLER(handler) { \
  char register_path[server_ctx.config->base_path_len + \
                     strlen(SOUP_HANDLER_PATH(handler)) + 1]; \
  memcpy(register_path, config->base_path, server_ctx.config->base_path_len); \
  memcpy(register_path + server_ctx.config->base_path_len, \
         SOUP_HANDLER_PATH(handler), strlen(SOUP_HANDLER_PATH(handler)) + 1); \
  soup_server_add_handler(server, SOUP_HANDLER_PATH(handler), handler, \
                          &server_ctx, NULL); \
}


/**@}*/

#endif /* DFCC_SERVER_HANDLER_COMMON_H */
