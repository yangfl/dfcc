#include <macro.h>

#include "prepost.h"
#include "local.h"
#include "remote.h"
#include "client.h"


int Client_start (struct Config *config) {
  return_if(Client_pre(config) == 0) 0;

  struct Result result = {0};
  int ret;

  if (Client_run_remotely(config, &result) == 0) {
    ret = 0;
  } else {
    ret = Client_run_locally(config, &result);
  }

  Client_post(config, &result);
  return ret;
}
