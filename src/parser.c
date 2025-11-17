#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <lex.h>
#include <parser.h>
#include <tree.h>

#define YANK(x) ++(x->current)
#define TOKEN_IS(tkn, t) assert((tkn)->type == t)

static enum node_type_t token_operator_type_to_node_type(enum token_type_t token_type) {
  switch (token_type) {
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

static int is_operator(const struct token_t *const tkn) {
  return tkn->type == TOKEN_TYPE_AND
    || tkn->type == TOKEN_TYPE_OR
    || tkn->type == TOKEN_TYPE_PIPE;
}

static int is_env_string(const struct token_list_t *const tkn) {
  return tkn->current->type == TOKEN_TYPE_RAW_STRING && strchr(tkn->current->literal, '=') != NULL;
}


void parse_redir(struct token_list_t *tkns, struct redir_node_t *node) {
  assert(is_redir(tkns->current));
  switch (tkns->current->type) {
  case TOKEN_TYPE_REDIR_IN_HERE_DOC:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->eod, tkns->current->literal, tkns->current->size);
    node->eod[tkns->current->size] = '\0';
    assert(input_flag_to_options(node->in) == 0);
    set_input_options(&node->in, REDIR_IN_FLAG_HERE_DOC);
    break;
  case TOKEN_TYPE_REDIR_IN_HERE_STRING:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->here_string, tkns->current, tkns->current->size);
    node->file_in[tkns->current->size] = '\0';
    assert(input_flag_to_options(node->in) == 0);
    set_input_options(&node->in, REDIR_IN_FLAG_HERE_STRING);
    break;
  case TOKEN_TYPE_REDIR_IN_FILE:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->file_in, tkns->current, tkns->current->size);
    node->file_in[tkns->current->size] = '\0';
    assert(input_flag_to_options(node->in) == 0);
    set_input_options(&node->in, REDIR_IN_FLAG_FILE);
    break;

  case TOKEN_TYPE_REDIR_OUT_TRUNC:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->file_out, tkns->current->literal, tkns->current->size);
    node->file_out[tkns->current->size] = '\0';
    assert(flag_to_options(node->out) == 0);
    set_options(&node->out, REDIR_OUT_FLAG_TRUNC);
    break;
  case TOKEN_TYPE_REDIR_OUT_APPEND:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->file_out, tkns->current->literal, tkns->current->size);
    node->file_out[tkns->current->size] = '\0';
    assert(flag_to_options(node->out) == 0);
    set_options(&node->out, REDIR_OUT_FLAG_APPEND);
    break;

  case TOKEN_TYPE_REDIR_ERR_APPEND:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->file_err, tkns->current->literal, tkns->current->size);
    node->file_err[tkns->current->size] = '\0';
    assert(flag_to_options(node->err) == 0);
    set_options(&node->err, REDIR_ERR_FLAG_APPEND);
    break;
  case TOKEN_TYPE_REDIR_ERR_TRUNC:
    YANK(tkns);
    assert(is_string(tkns->current));
    memcpy(node->file_err, tkns->current->literal, tkns->current->size);
    node->file_err[tkns->current->size] = '\0';
    assert(flag_to_options(node->err) == 0);
    set_options(&node->err, REDIR_ERR_FLAG_TRUNC);
    break;

  default:
    assert(0 && "UNREACHABLE");
  }
  YANK(tkns);
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
      YANK(tkns);
    }
  }
  cmd->env[total_env] = NULL;

  assert(is_string(tkns->current));
  memcpy(cmd->executable, tkns->current->literal, tkns->current->size);
  cmd->executable[tkns->current->size] = '\0';
  cmd->argv[cmd->argc++] = cmd->executable;
  YANK(tkns);

  while (is_string(tkns->current) || is_redir(tkns->current)) {
    if (is_redir(tkns->current)) {
      if (redir_node == NULL) {
	redir_node = malloc(sizeof(struct redir_node_t));
      }

      parse_redir(tkns, redir_node);

    }
    else if (is_string(tkns->current)){
      cmd->argv[cmd->argc++] = tkns->current->literal;
      YANK(tkns);
    }
    else {
      break;
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

struct node_t *parse_operator(struct token_list_t *tkns) {
  struct node_t *node;

  assert(is_operator(tkns->current));

  node = malloc(sizeof(struct node_t));
  node->type = token_operator_type_to_node_type(tkns->current->type);

  node->left_node = NULL;
  node->right_node = NULL;

  YANK(tkns);
  return node;
}
struct node_t *parse(const char *const string) {
  struct token_list_t *tkns;
  struct node_t *op_stack[10];
  struct node_t *cmd_stack[10];
  int op_stack_idx, cmd_stack_idx, i, k;

  op_stack_idx = 0;
  cmd_stack_idx = 0;
  tkns = lex(string);
  while (tkns->current->type != TOKEN_TYPE_EOF) {
    struct node_t *cmd_node;
    struct node_t *op_node;
    if (is_operator(tkns->current)) {
      op_node = parse_operator(tkns);
      op_stack[op_stack_idx++] = op_node;
    }

    cmd_node = parse_cmd(tkns);
    cmd_stack[cmd_stack_idx++] = cmd_node;
  }

  free_token_list(tkns);

  assert(op_stack_idx + 1 == cmd_stack_idx);
#ifdef DEBUG
  printf("%d ops, %d cmds\n", op_stack_idx, cmd_stack_idx);
#endif

  k=2;
  for (i=0; i<op_stack_idx; ++i) {
    struct node_t *op_node;

    op_node = op_stack[i];
    if (i == 0) {
      struct node_t *cmd_node_1;
      struct node_t *cmd_node_2;
      cmd_node_1 = cmd_stack[0];
      cmd_node_2 = cmd_stack[1];

      op_node->left_node = cmd_node_1;
      op_node->right_node = cmd_node_2;
    }
    else {
      op_node->left_node = op_stack[i-1];
      op_node->right_node = cmd_stack[k++];
    }
  }

  if (op_stack_idx == 0) {
    assert(cmd_stack_idx == 1);
    return cmd_stack[0];
  }
  return op_stack[op_stack_idx-1];
}
