#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lex.h>
#include <cmd.h>

int main() {
  char *string = "/usr/bin/ls -l /home/fuji";
  struct node_t *node = parse(string);
  printf_tree(node, 0);
  run(node);

  return EXIT_SUCCESS;
}
