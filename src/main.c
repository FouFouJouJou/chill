#include <stdio.h>
#include <stdlib.h>
#include <parser.h>

int main() {
  parse("echo \"Hello $USER\"");
  return EXIT_SUCCESS;
}
