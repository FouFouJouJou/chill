#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <parser.h>
#include <lex.h>
#include <env.h>
#include <cmd.h>
#include <builtin.h>
#include <prompt.h>
#include <history.h>
#include <job.h>

extern int exit_code;

int main() {

  char string[1<<8];
  init_environ();
  init_free_list();
  sync_history();

  while (1) {
    struct node_t *node;
    int size;

    size = prompt(string);

    if (size == 0) {
      continue;
    }

    node = parse(string);
    append_cmd(string);
    memset(string, 0, sizeof(string));

    schedule(node);
  }
  return EXIT_SUCCESS;
}
