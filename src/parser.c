#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <lex.h>
#include <parser.h>
#include <tree.h>

#define YANK(x) ++x
#define TOKEN_IS(tkn, t) assert((tkn)->type == t)

static enum node_type_t token_symbol_type_to_node_type(enum token_type_t token_type) {
  switch(token_type) {
  case TOKEN_TYPE_AND:
    return NODE_TYPE_AND;
  case TOKEN_TYPE_PIPE:
    return NODE_TYPE_PIPE;
  case TOKEN_TYPE_OR:
    return NODE_TYPE_OR;
  default:
    fprintf(stderr, "%s not a symbol\n", token_type_to_string(token_type));
    exit(81);
  }
}

static int is_redir(const struct token_t *const tkn) {
  return
    tkn->type == TOKEN_TYPE_REDIR_IN_HERE_STRING
    || tkn->type == TOKEN_TYPE_REDIR_IN_FILE
    || tkn->type == TOKEN_TYPE_REDIR_IN_HERE_DOC
    || tkn->type == TOKEN_TYPE_REDIR_OUT_APPEND
    || tkn->type == TOKEN_TYPE_REDIR_OUT_TRUNC
    || tkn->type == TOKEN_TYPE_REDIR_ERR_APPEND
    || tkn->type == TOKEN_TYPE_REDIR_ERR_TRUNC;
}

static int is_string(const struct token_t *const tkn) {
  return
    tkn->type == TOKEN_TYPE_RAW_STRING
    || tkn->type == TOKEN_TYPE_DOUBLEQ_STRING
    || tkn->type == TOKEN_TYPE_SINGLEQ_STRING;
}

static int is_symbol(const struct token_t *const tkn) {
  return !is_string(tkn);
}

static int is_env_string(const struct token_list_t *const tkn) {
  assert(tkn->current->type == TOKEN_TYPE_RAW_STRING);
  return strchr(tkn->current->literal, '=') != NULL;
}

struct node_t *parse_symbol(struct token_list_t *tkns) {
  struct double_node_t *double_node;
  struct node_t *node;

  assert(is_symbol(tkns->current));

  double_node = malloc(sizeof(struct double_node_t));
  node = malloc(sizeof(struct node_t *));

  node->type = token_symbol_type_to_node_type(tkns->current->type);
  node->node = (void*) node;

  double_node->left_node = NULL;
  double_node->right_node = NULL;

  YANK(tkns->current);

  return node;
}

void parse_redir(struct token_list_t *tkns, struct redir_node_t *node) {
  assert(is_redir(tkns->current));
  switch(tkns->current->type) {
  case TOKEN_TYPE_REDIR_IN_HERE_DOC:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->eod, tkns->current, tkns->current->size);
    node->file_in[tkns->current->size] = '\0';
    assert(input_flag_to_options(node->in) == 0);
    set_input_options(&node->in, REDIR_IN_FLAG_HERE_DOC);
    break;
  case TOKEN_TYPE_REDIR_IN_HERE_STRING:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->here_string, tkns->current, tkns->current->size);
    node->file_in[tkns->current->size] = '\0';
    assert(input_flag_to_options(node->in) == 0);
    set_input_options(&node->in, REDIR_IN_FLAG_HERE_STRING);
    break;
  case TOKEN_TYPE_REDIR_IN_FILE:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->file_in, tkns->current, tkns->current->size);
    node->file_in[tkns->current->size] = '\0';
    assert(input_flag_to_options(node->in) == 0);
    set_input_options(&node->in, REDIR_IN_FLAG_FILE);
    break;

  case TOKEN_TYPE_REDIR_OUT_TRUNC:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->file_out, tkns->current->literal, tkns->current->size);
    node->file_out[tkns->current->size] = '\0';
    assert(flag_to_options(node->out) == 0);
    set_options(&node->out, REDIR_OUT_FLAG_TRUNC);
    break;
  case TOKEN_TYPE_REDIR_OUT_APPEND:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->file_out, tkns->current->literal, tkns->current->size);
    node->file_out[tkns->current->size] = '\0';
    assert(flag_to_options(node->out) == 0);
    set_options(&node->out, REDIR_OUT_FLAG_APPEND);
    break;

  case TOKEN_TYPE_REDIR_ERR_APPEND:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->file_err, tkns->current->literal, tkns->current->size);
    node->file_err[tkns->current->size] = '\0';
    assert(flag_to_options(node->err) == 0);
    set_options(&node->err, REDIR_ERR_FLAG_APPEND);
    break;
  case TOKEN_TYPE_REDIR_ERR_TRUNC:
    YANK(tkns->current);
    assert(is_string(tkns->current));
    memcpy(node->file_err, tkns->current->literal, tkns->current->size);
    node->file_err[tkns->current->size] = '\0';
    assert(flag_to_options(node->err) == 0);
    set_options(&node->err, REDIR_ERR_FLAG_TRUNC);
    break;

  default:
    assert(0 && "UNREACHABLE");
  }
  YANK(tkns->current);
}

struct node_t *parse_cmd(struct token_list_t *tkns) {
  struct cmd_t *cmd;
  struct node_t *node;

  struct cmd_node_t *cmd_node;
  struct redir_node_t *redir_node;

  int total_env;

  cmd = malloc(sizeof(struct cmd_t));
  cmd->argc = 0;

  cmd_node = malloc(sizeof(struct cmd_node_t));
  cmd_node->cmd = cmd;

  node = malloc(sizeof(struct node_t));
  node->type = NODE_TYPE_CMD;
  node->node = (void*) cmd_node;

  redir_node = NULL;
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
    if (is_redir(tkns->current)) {
      if (redir_node == NULL) {
	redir_node = malloc(sizeof(struct redir_node_t));
      }

      parse_redir(tkns, redir_node);

    } else {
      cmd->argv[cmd->argc++] = tkns->current->literal;
      YANK(tkns->current);
    }
  }
  cmd->argv[cmd->argc] = NULL;

  if (redir_node != NULL) {
    struct node_t *child_node = malloc(sizeof(struct node_t));
    child_node->type = NODE_TYPE_CMD;
    child_node->node = (void *)cmd_node;
    redir_node->node = child_node;
    node->node = (void*) redir_node;
    node->type = NODE_TYPE_REDIR;
  }
  return node;
}

struct node_t *parse(const char *const string) {
  struct token_list_t *tkns;

  tkns = lex(string);
  printf_tree(parse_cmd(tkns), 2);

  free(tkns);
  return NULL;
}
