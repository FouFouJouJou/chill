#ifndef __TREE_H__
#define __TREE_H__
#include <cmd.h>
#include <stdint.h>

enum node_type_t {
  NODE_TYPE_CMD
  , NODE_TYPE_AND
  , NODE_TYPE_OR
  , NODE_TYPE_REDIR
};

struct redir_node_t {
  /* redirection flag: 00000000000000000000000000000000. */
  /* MSB indicates if redirection is on or off. */
  /* Rest of the flag is the file descriptor. */
  uint32_t in, out, err;
  char file_in[1<<8], file_out[1<<8], file_err[1<<8];
  struct node_t *node;
};

struct cmd_node_t {
  struct cmd_t *cmd;
};

struct double_node_t {
  struct node_t *left_node;
  struct node_t *right_node;
};

struct node_t {
  enum node_type_t type;
  void *node;
};

ssize_t run(const struct node_t *const node);
#endif
