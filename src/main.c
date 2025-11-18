#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser.h>
#include <lex.h>
#include <env.h>
#include <cmd.h>
#include <prompt.h>
#include <history.h>

extern int exit_code;
extern struct environ_t environ_;
extern struct history_t history;

int main() {
  char string[1<<8];
  init_environ();
  read_history(&history);

  while (1) {
    struct node_t *node;
    prompt(string);

    node = parse(string);
#ifdef DEBUG
    printf_tree(node, 0);
#endif
    append_cmd(string, &history);
    exit_code = run(node);
    memset(string, 0, sizeof(string));
    free_tree(node);
  }
  return EXIT_SUCCESS;
}
