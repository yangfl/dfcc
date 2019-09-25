#include "local_hash.h"


int do_client (int cc_argc, char *cc_argv[]) {
  local_hash_init();

  char *fn = "a.out";
  printf("%lx\n",local_hash_get(fn)->hash);

  local_hash_destory();
  return 0;
}
