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
extern struct history_t history;

void loop_() {
}

int main() {
  char string[1<<8];
  read_history(&history);

  while (1) {
    struct node_t *node;
    prompt(string);
    node = parse(string);
#ifdef DEBUG
    printf_tree(node, 0);
#endif
    exit_code = run(node);
    append_cmd(string, &history);
    memset(string, 0, sizeof(string));
    free_tree(node);
  }
  return EXIT_SUCCESS;
}
