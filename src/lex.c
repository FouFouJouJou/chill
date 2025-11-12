#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <lex.h>

const char *token_type_to_string(enum token_type_t type) {
  switch(type) {
  case TOKEN_TYPE_AND:
    return "TOKEN_TYPE_AND";
  case TOKEN_TYPE_REDIR_IN_HERE_STRING:
    return "TOKEN_TYPE_REDIR_IN_HERE_STRING";
  case TOKEN_TYPE_SINGLEQ_STRING:
    return "TOKEN_TYPE_SINGLEQ_STRING";
  case TOKEN_TYPE_DOUBLEQ_STRING:
    return "TOKEN_TYPE_DOUBLEQ_STRING";
  case TOKEN_TYPE_OR:
    return "TOKEN_TYPE_OR";
  case TOKEN_TYPE_PIPE:
    return "TOKEN_TYPE_PIPE";
  case TOKEN_TYPE_REDIR_IN_HERE_DOC:
    return "TOKEN_TYPE_REDIR_IN_HERE_DOC";
  case TOKEN_TYPE_REDIR_IN_FILE:
    return "TOKEN_TYPE_REDIR_IN_FILE";
  case TOKEN_TYPE_REDIR_OUT_APPEND:
    return "TOKEN_TYPE_REDIR_OUT_APPEND";
  case TOKEN_TYPE_REDIR_OUT_TRUNC:
    return "TOKEN_TYPE_REDIR_OUT_TRUNC";
  case TOKEN_TYPE_REDIR_ERR_APPEND:
    return "TOKEN_TYPE_REDIR_ERR_APPEND";
  case TOKEN_TYPE_REDIR_ERR_TRUNC:
    return "TOKEN_TYPE_REDIR_ERR_TRUNC";
  case TOKEN_TYPE_RAW_STRING:
    return "TOKEN_TYPE_RAW_STRING";
  default:
    exit(81);
  }
}

struct token_t *symbol_tknzr(const char *const string, const struct tknzr_t *const tknzr) {
  struct token_t *tkn;
  size_t literal_size;
  tkn = NULL;
  literal_size = strlen(tknzr->literal);

  if (!strncmp(string, tknzr->literal, literal_size)) {
    tkn = malloc(sizeof(struct token_t));
    memcpy(tkn->literal, string, literal_size);
    tkn->literal[literal_size] = '\0';
    tkn->size = literal_size;
    tkn->type = tknzr->type;
  }

  return tkn;
}

struct token_t *string_tknzr(const char *const string) {
  struct token_t *tkn;
  char quotes;
  const char *string_p;
  tkn = NULL;
  if (*string == '"' || *string == '\'') {
    char *chr;
    quotes = *string;
    string_p = string+1;
    while(*string_p != '\0') {
      chr = strchr(string_p, quotes);
      if (chr == NULL) {
	break;
      }
      if (*(chr-1) == '\\') {
	string_p = chr+1;
      }
      else {
	tkn = malloc(sizeof(struct token_t));
	tkn->size = chr - string + 1;
	tkn->type = quotes == '"' ? TOKEN_TYPE_DOUBLEQ_STRING : TOKEN_TYPE_SINGLEQ_STRING;
	memcpy(tkn->literal, string, tkn->size);
	break;
      }
    }
  }
  else {
    int size;
    size = strcspn(string, " &><|");
    tkn = malloc(sizeof(struct token_t));
    tkn->size = size;
    tkn->type = TOKEN_TYPE_RAW_STRING;
    memcpy(tkn->literal, string, tkn->size);
  }

  return tkn;
}

void append_tknzr(struct tknzr_t tknzrs[], char *literal, enum token_type_t type, size_t *count) {
  tknzrs[*count].type = type;
  tknzrs[*count].literal = literal;
  (*count)+=1;
}

struct token_t make_eof_token() {
  struct token_t tkn;
  tkn.type = TOKEN_TYPE_EOF;
  return tkn;
}

void printf_token(const struct token_t *const token) {
  printf("token_t(literal=`%s`, size=%lu, type=%s)\n", token->literal, token->size, token_type_to_string(token->type));
}

struct token_t *lex(const char *const input) {
  struct token_t *tkns;
  struct tknzr_t tknzrs[1<<4];
  struct token_t *tkn;
  const char *input_ptr;
  size_t i;
  size_t total_tknzrs;
  size_t total_tkns;

  total_tknzrs = 0;
  total_tkns = 0;
  input_ptr = input;

  tkns = malloc(sizeof(struct token_t) * TOTAL_SUPPORTED_TOKENS);

  append_tknzr(tknzrs,"<<<", TOKEN_TYPE_REDIR_IN_HERE_STRING, &total_tknzrs);
  append_tknzr(tknzrs,"2>>", TOKEN_TYPE_REDIR_ERR_APPEND, &total_tknzrs);

  append_tknzr(tknzrs,"<<", TOKEN_TYPE_REDIR_IN_HERE_DOC, &total_tknzrs);
  append_tknzr(tknzrs,">>", TOKEN_TYPE_REDIR_OUT_APPEND, &total_tknzrs);
  append_tknzr(tknzrs,"2>", TOKEN_TYPE_REDIR_ERR_TRUNC, &total_tknzrs);
  append_tknzr(tknzrs,"&&", TOKEN_TYPE_AND, &total_tknzrs);
  append_tknzr(tknzrs,"||", TOKEN_TYPE_OR, &total_tknzrs);

  append_tknzr(tknzrs,"|", TOKEN_TYPE_PIPE, &total_tknzrs);
  append_tknzr(tknzrs,">", TOKEN_TYPE_REDIR_OUT_TRUNC, &total_tknzrs);
  append_tknzr(tknzrs,"<", TOKEN_TYPE_REDIR_IN_FILE, &total_tknzrs);

  while (*input_ptr != '\0') {
    input_ptr += strspn(input_ptr, " ");
    for (i=0; i<total_tknzrs; ++i) {
      if ((tkn = symbol_tknzr(input_ptr, tknzrs+i)) != NULL) {
	goto advance;
      }
    }
    tkn = string_tknzr(input_ptr);
    assert(tkn != NULL);
  advance:
    total_tkns+=1;
    tkns[total_tkns-1] = *tkn;
    input_ptr += tkn->size;
    free(tkn);
  }

  tkns[total_tkns] = make_eof_token();
  return tkns;
}
