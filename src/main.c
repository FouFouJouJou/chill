#include <stdio.h>
#include <stdlib.h>
#include <parser.h>

int main() {
  parse("NAME=FOUFOU echo $NAME");
  return EXIT_SUCCESS;
}
