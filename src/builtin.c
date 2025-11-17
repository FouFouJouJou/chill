#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <builtin.h>
/* TODO: implement which to locate executables */

static int cd(int argc, char **argv, char **env) {
  (void) env;
  assert(argc == 2);
  if (chdir(argv[1]) == -1) {
    return errno;
  }

  return 0;
}

static int echo(int argc, char **argv, char **env) {
  int i;
  (void) env;
  for (i=0; i<argc-1; ++i) {
    printf("%s", (argv+1)[i]);
    if (i+1 != argc) {
      printf(" ");
    }
  }
  printf("\n");
  return 0;
}

builtin_t cmd_to_builtin(const char *const cmd) {
  builtin_t fn = NULL;
  if (!strncmp(cmd, "echo", 4)) {
    fn = echo;
  }
  if (!strncmp(cmd, "cd", 2)) {
    fn = cd;
  }

  return fn;
}
