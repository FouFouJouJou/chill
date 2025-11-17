#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lex.h>
#include <cmd.h>

int main() {
  int exit_code;
  char *string = "wc /Users/foufou/.vimrc";
  struct node_t *node = parse(string);
#ifdef DEBUG
  printf_tree(node, 0);
#endif
  exit_code = run(node);
  printf("exit_code: %d\n", exit_code);
  free_tree(node);

  return EXIT_SUCCESS;
}
