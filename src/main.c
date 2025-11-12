#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lex.h>
#include <cmd.h>
#include <tree.h>

int main() {
  struct token_t *tkn;
  struct token_t *tkns;

  tkns = lex("NAME=FouFou echo $NAME&&ls $HOME/.vimrc");
  for (tkn = tkns; tkn->type != TOKEN_TYPE_EOF; ++tkn) {
    printf_token(tkn);
  }

  free(tkns);
  return EXIT_SUCCESS;
}
