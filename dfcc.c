#include <getopt.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception.h>

#include "macro.h"
#include "client.h"
#include "server.h"

#include "dfcc.h"


static void usage (const char *progname) {
  printf("usage: %s [options]\n\n", progname);
  printf(
"options:\n"
"blah\n"
);
}


static void show_version (const char *progname) {
  printf("%s %s\n",
          progname,
          DFCC_VERSION);
}


static bool is_symlinked (const char *progname) {
  char *executable_filename = realpath(progname, NULL);
  if unlikely (executable_filename == NULL) {
    handle_error("realpath");
  }
  char *called_filename = strdup(progname);
  if unlikely (called_filename == NULL) {
    handle_error("strdup");
  }
  bool symlinked = strcmp(basename(executable_filename), basename(called_filename)) != 0;
  free(executable_filename);
  free(called_filename);
  return symlinked;
}


const struct option long_options[] = {
  {"server",  no_argument,       0,  'S'},
  {"port",  required_argument,       0,  'p'},
  {"foreground",  no_argument,       0,  'f'},
  {"debug",  no_argument,       0,  'd'},
  {"version",  no_argument,       0,  'v'},
  {"help",  no_argument,       0,  'h'},
  {0,         0,                 0,  0 }
};


int main (int argc, char *argv[]) {
  Quick;
  return 0;

  bool symlinked = is_symlinked(argv[0]);
  bool server_mode = false;
  unsigned short port = DFCC_PORT;
  bool foreground = false;
  bool debug = false;

  int wrapper_argc = argc;

  if (symlinked) {
    for (wrapper_argc--; wrapper_argc > 1; wrapper_argc--) {
      if (strcmp(argv[wrapper_argc], "--") == 0) {
        break;
      }
    }
  }

  while (1) {
    int option_index = 0;

    int c = getopt_long(wrapper_argc, argv, "-Sp:fdvh", long_options, &option_index);
    if (c == -1) {
      break;
    }
    if (c == 1) {
      optind--;
      break;
    }

    switch (c) {
      case 0:
        printf("option %s", long_options[option_index].name);
        if (optarg)
           printf(" with arg %s", optarg);
        printf("\n");
        break;

      case 'S':
        server_mode = true;
        break;

      case 'p':
        port = atoi(optarg);
        break;

      case 'f':
        foreground = true;
        break;

      case 'd':
        debug = true;
        break;

      case 'v':
        show_version(argv[0]);
        return EXIT_SUCCESS;

      case 'h':
        usage(argv[0]);
        return EXIT_SUCCESS;

      default:
        printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (server_mode) {
    do_server(port);
  } else {
    int cc_argc;
    if (symlinked) {
      cc_argc = argc - wrapper_argc;
    } else {
      cc_argc = argc - optind;
    }

    char **cc_argv = malloc(sizeof(char *) * cc_argc);
    if unlikely (cc_argv == NULL) {
      handle_error("malloc");
    }

    if (symlinked) {
      cc_argv[0] = argv[0];
      for (int i = 1; i < cc_argc; i++) {
        cc_argv[i] = argv[wrapper_argc + i];
      }
    } else {
      for (int i = 0; i < cc_argc; i++) {
        cc_argv[i] = argv[optind + i];
      }
    }

    do_client(cc_argc, cc_argv);
  }

  return EXIT_SUCCESS;
}

