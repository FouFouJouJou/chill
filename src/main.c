#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lex.h>
#include <cmd.h>
#include <prompt.h>

int main() {
  char string[1<<8];
  int exit_code;

  while (1) {
    struct node_t *node;
    prompt(string);
    node = parse(string);
#ifdef DEBUG
    printf_tree(node, 0);
#endif
    exit_code = run(node);
    printf("exit_code: %d\n", exit_code);
    free_tree(node);
  }

  return EXIT_SUCCESS;
}
