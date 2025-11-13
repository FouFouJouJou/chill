#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <lex.h>
#include <parser.h>
#include <tree.h>

#define YANK(x) ++x
#define TOKEN_IS(tkn, t) assert((tkn)->type == t)

static int is_string(const struct token_t *const tkn) {
  return tkn->type == TOKEN_TYPE_RAW_STRING || tkn->type == TOKEN_TYPE_DOUBLEQ_STRING || tkn->type == TOKEN_TYPE_SINGLEQ_STRING;
}

static int is_env_string(const struct token_list_t *const tkn) {
  assert(tkn->current->type == TOKEN_TYPE_RAW_STRING);
  return strchr(tkn->current->literal, '=') != NULL;
}

void parse_and(struct token_list_t *tkns) {
  printf("------- AND -------\n");
  TOKEN_IS(tkns->current, TOKEN_TYPE_AND);
  printf_token(tkns->current);
  YANK(tkns->current);
}

struct node_t *parse_cmd(struct token_list_t *tkns) {
  struct cmd_t *cmd;
  struct node_t *node;
  struct cmd_node_t *cmd_node;

  int total_env;

  cmd = malloc(sizeof(struct cmd_t));
  cmd->argc = 0;

  cmd_node = malloc(sizeof(struct cmd_node_t));
  cmd_node->cmd = cmd;

  node = malloc(sizeof(struct node_t));
  node->type = NODE_TYPE_CMD;
  node->node = (void*) cmd_node;

  total_env = 0;
  if (is_env_string(tkns)) {
    while(is_env_string(tkns)) {
      cmd->env[total_env++] = tkns->current->literal;
      YANK(tkns->current);
    }
  }
  cmd->env[total_env] = NULL;

  assert(is_string(tkns->current));
  memcpy(cmd->executable, tkns->current->literal, tkns->current->size);
  cmd->executable[tkns->current->size] = '\0';
  cmd->argv[cmd->argc++] = cmd->executable;
  YANK(tkns->current);

  while (tkns->current->type != TOKEN_TYPE_EOF) {
    cmd->argv[cmd->argc++] = tkns->current->literal;
    printf_token(tkns->current);
    YANK(tkns->current);
  }
  cmd->argv[cmd->argc] = NULL;

  printf_cmd(cmd);
  return node;
}

struct node_t *parse(const char *const string) {
  struct token_list_t *tkns;

  tkns = lex(string);
  parse_cmd(tkns);

  free(tkns);
  return NULL;
}
