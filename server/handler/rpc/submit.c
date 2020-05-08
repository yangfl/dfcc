#include <libsoup/soup.h>

#include "common/macro.h"
#include "common/wrapper/soup.h"
#include "../../protocol.h"
#include "../../log.h"
#include "submit.h"


void Server_rpc_submit (
    struct ServerContext *server_ctx, struct Session *session,
    SoupMessage *msg, GVariant *param) {
  char **cc_argv;
  char **cc_envp;
  const char *cc_working_directory;
  GVariantIter *settings_iter;
  g_variant_get(param, "(^a&s^a&s&sa{sv})",
                &cc_argv, &cc_envp, &cc_working_directory, &settings_iter);

  GError *error = NULL;
  struct HookedProcess *p = HookedProcessGroup_new_job(
    (struct HookedProcessGroup *) session, cc_argv, cc_envp, NULL, NULL, &error);
  should (p != NULL) otherwise {
    g_log(DFCC_SERVER_NAME, G_LOG_LEVEL_INFO,
          "Cannot create job for session %x: %s",
          session->hgid, error->message);
    soup_xmlrpc_message_set_fault(msg, 0, error->message);
    if (error->domain == DFCC_SPAWN_ERROR &&
        SOUP_STATUS_IS_SERVER_ERROR(error->code)) {
      soup_message_set_status(msg, error->code);
    }
    g_error_free(error);
  }

  g_free(cc_argv);
  g_free(cc_envp);
  g_variant_iter_free(settings_iter);
  g_variant_unref(param);

  return_if_fail(p != NULL);
  soup_xmlrpc_message_set_response_e(
    msg, g_variant_new_uint32(p->pid), DFCC_SERVER_NAME);
}
