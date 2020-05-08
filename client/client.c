#include "common/macro.h"
#include "ccargs/ccargs.h"
#include "log.h"
#include "prepost.h"
#include "local.h"
#include "remote.h"
#include "client.h"


int Client_start (struct Config *config) {
  return_if(Client_pre(config) == 0) 0;

  struct Result result = {0};
  int ret = 1;

  char **remote_argv = g_strdupv(config->cc_argv);
  char **remote_envp = g_strdupv(config->cc_envp);
  if likely (CCargs_can_run_remotely(&remote_argv, &remote_envp)) {
    if (Client_run_remotely(config, &result, remote_argv, remote_envp) == 0) {
      ret = 0;
    } else {
      g_log(DFCC_CLIENT_NAME, G_LOG_LEVEL_WARNING, "Remote compile failed, fallback to local");
    }
  }
  g_strfreev(remote_argv);
  g_strfreev(remote_envp);

  if (ret != 0) {
    ret = Client_run_locally(config, &result);
  }

  Client_post(config, &result);
  return ret;
}
