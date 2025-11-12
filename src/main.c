#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lex.h>
#include <cmd.h>
#include <tree.h>

int main() {
  lex("NAME=FouFou echo $NAME&&ls $HOME/.vimrc");
  return EXIT_SUCCESS;
}
