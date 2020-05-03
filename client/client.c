#include <macro.h>

#include "local.h"
#include "remote.h"
#include "client.h"


int Client_start (struct Config *config) {
  if (Client_run_remotely(config) == 0) {
    return 0;
  }

  return Client_run_locally(config);
}
