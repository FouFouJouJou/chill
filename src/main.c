#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lex.h>
#include <cmd.h>

int main() {
  char *string = "dir=/Users/foufou /bin/ls $dir";
  struct node_t *node = parse(string);
#ifdef DEBUG
  printf_tree(node, 0);
#endif
  run(node);
  free_tree(node);

  return EXIT_SUCCESS;
}
