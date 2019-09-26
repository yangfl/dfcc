#include <macro.h>

#include "ccargs.h"
#include "local.h"
#include "remote.h"
#include "client.h"


int Client_start (struct Config *config) {
  do_once {
    if (!Client_can_run_remotely(&config->cc_argv)) {
      break;
    }

    if (Client_run_remotely(config) != 0) {
      break;
    }

    return 0;
  }

  return Client_run_locally(config);
}
