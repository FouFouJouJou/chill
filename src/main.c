#include <stdio.h>
#include <stdlib.h>
#include <parser.h>

int main() {
  struct node_t *node = parse("/usr/bin/wc <<LOL >> /tmp/out.txt && /usr/bin/ls");
  printf_tree(node, 0);
  run(node);
  return EXIT_SUCCESS;
}
