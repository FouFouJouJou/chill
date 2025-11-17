#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parser.h>
#include <lex.h>
#include <env.h>
#include <cmd.h>
#include <prompt.h>

extern int exit_code;

int main() {
  char string[1<<8];

  while (1) {
    struct node_t *node;
    prompt(string);
    node = parse(string);
#ifdef DEBUG
    printf_tree(node, 0);
#endif
    exit_code = run(node);
    memset(string, 0, sizeof(string));
    free_tree(node);
  }

  return EXIT_SUCCESS;
}
