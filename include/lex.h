#ifndef __LEX_H__
#define __LEX_H__


enum token_type_t {
  TOKEN_TYPE_RAW_STRING,
  TOKEN_TYPE_SINGLEQ_STRING,
  TOKEN_TYPE_DOUBLEQ_STRING,
  TOKEN_TYPE_AND,
  TOKEN_TYPE_OR,
  TOKEN_TYPE_PIPE,
  TOKEN_TYPE_REDIR_IN_HERE_DOC,
  TOKEN_TYPE_REDIR_IN_HERE_STRING,
  TOKEN_TYPE_REDIR_IN_FILE,
  TOKEN_TYPE_REDIR_OUT_APPEND,
  TOKEN_TYPE_REDIR_OUT_TRUNC,
  TOKEN_TYPE_REDIR_ERR_APPEND,
  TOKEN_TYPE_REDIR_ERR_TRUNC,
  TOKEN_TYPE_TOTAL
};

struct token_t {
  enum token_type_t type;
  char literal[1<<8];
  size_t size;
};

struct tknzr_t {
  char *literal;
  enum token_type_t type;
};

typedef struct token_t *(*tknzr_fn)(const char *const string, const struct tknzr_t *const tknzr);
const char *token_type_to_string(enum token_type_t type);
void lex(const char *const input);

#endif
