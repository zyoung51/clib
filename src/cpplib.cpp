
//
// clib.c
//
// Copyright (c) 2012-2014 clib authors
// MIT licensed
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern "C" {
#include "trim/trim.h"
#include "asprintf/asprintf.h"
#include "which/which.h"
#include "str-flatten/str-flatten.h"
#include "debug/debug.h"
}
#include "config.h"

debug_t debugger;

static const char *usage =
  "\n"
  "  cpplib <command> [options]\n"
  "\n"
  "  Options:\n"
  "\n"
  "    -h, --help     Output this message\n"
  "    -v, --version  Output version information\n"
  "\n"
  "  Commands:\n"
  "\n"
  "    install [name...]  Install one or more packages\n"
  "    search [query]     Search for packages\n"
  "    help <cmd>         Display help for cmd\n"
  "";

#define format(...) ({                               \
  if (-1 == asprintf(__VA_ARGS__)) {                 \
    rc = 1;                                          \
    fprintf(stderr, "Memory allocation failure\n");  \
    goto cleanup;                                    \
  }                                                  \
})

int
main(int argc, const char **argv) {
  char *cmd = NULL;
  char *args = NULL;
  char *command = NULL;
  char *command_with_args = NULL;
  char *bin = NULL;
  int rc = 1;

  debug_init(&debugger, "clib");

  // usage
  if (NULL == argv[1]
   || 0 == strncmp(argv[1], "-h", 2)
   || 0 == strncmp(argv[1], "--help", 6)) {
    printf("%s\n", usage);
    return 0;
  }

  // version
  if (0 == strncmp(argv[1], "-v", 2)
   || 0 == strncmp(argv[1], "--version", 9)) {
    printf("%s\n", PACKAGE_VERSION);
    return 0;
  }

  // unknown
  if (0 == strncmp(argv[1], "--", 2)) {
    fprintf(stderr, "Unknown option: \"%s\"\n", argv[1]);
    return 1;
  }

  // sub-command
  cmd = strdup(argv[1]);
  if (NULL == cmd) {
    fprintf(stderr, "Memory allocation failure\n");
    return 1;
  }
  cmd = trim(cmd);

  if (0 == strcmp(cmd, "help")) {
    if (argc >= 3) {
      free(cmd);
      cmd = strdup(argv[2]);
      args = strdup("--help");
    } else {
      fprintf(stderr, "Help command required.\n");
      goto cleanup;
    }
  } else {
    if (argc >= 3) {
      args = str_flatten(argv, 2, argc);
      if (NULL == args) goto cleanup;
    }
  }
  debug(&debugger, "args: %s", args);

#ifdef _WIN32
  format(&command, "cpplib-%s.exe", cmd);
#else
  format(&command, "cpplib-%s", cmd);
#endif
  debug(&debugger, "command '%s'", cmd);

  bin = which(command);
  if (NULL == bin) {
    fprintf(stderr, "Unsupported command \"%s\"\n", cmd);
    goto cleanup;
  }

#ifdef _WIN32
  for (char *p = bin; *p; p++)
    if (*p == '/') *p = '\\';
#endif

  if (args) {
    format(&command_with_args, "%s %s", bin, args);
  } else {
    format(&command_with_args, "%s", bin);
  }

  debug(&debugger, "exec: %s", command_with_args);

  rc = system(command_with_args);
  debug(&debugger, "returned %d", rc);
  if (rc > 255) rc = 1;

cleanup:
  free(cmd);
  free(args);
  free(command);
  free(command_with_args);
  free(bin);
  return rc;
}
