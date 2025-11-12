#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lex.h>
#include <cmd.h>
#include <tree.h>

int main() {
  lex("2>>>><<\"I love the \\'way you lie\"");
  return EXIT_SUCCESS;
}
