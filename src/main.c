#include <stdio.h>
#include <stdlib.h>
#include <parser.h>

int main() {
  parse("NAME=FouFou SURNAME=JouJou echo >> out.txt $NAME && echo hello");
  return EXIT_SUCCESS;
}
