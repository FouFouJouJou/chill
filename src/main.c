#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lex.h>
#include <cmd.h>

int main() {
  char *string = "/bin/ls $dir";
  struct node_t *node = parse(string);
  printf_tree(node, 0);
  run(node);
  free_tree(node);

  return EXIT_SUCCESS;
}
