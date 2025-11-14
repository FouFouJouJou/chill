#ifndef __TREE_H__
#define __TREE_H__
#include <stdlib.h>
#include <cmd.h>
#include <stdint.h>
#define REDIR_IN_FLAG_FILE 4
#define REDIR_IN_FLAG_HERE_STRING 6
#define REDIR_IN_FLAG_HERE_DOC 5

#define REDIR_OUT_FLAG_APPEND 2
#define REDIR_OUT_FLAG_TRUNC 3

#define REDIR_ERR_FLAG_APPEND 2
#define REDIR_ERR_FLAG_TRUNC 3

enum node_type_t {
  NODE_TYPE_CMD
  , NODE_TYPE_AND
  , NODE_TYPE_OR
  , NODE_TYPE_REDIR
  , NODE_TYPE_PIPE
};

/* Redirection flag: 00000000000000000000000000000000. */
/* MSB indicates if redirection is on or off. */
struct redir_node_t {
  /* Most significant 3 bits control behaviour for in flags */
  /* 000 => No redirection */
  /* 100 => Redirection to `file_in` */
  /* 110 => Redirection to `here_string` */
  /* 101 => Redirection to here doc */
  /* 111 => Redirection to here doc (UNUSED)*/
  flag_t in;

  /* Most significant 2 bits control behaviour for out and err flags */
  /* 00 => No redirection */
  /* 10 => Redirection to `file_(out|err)` in append mode */
  /* 11 => Redirection to `file_(out|err)` in trunc mode */
  flag_t out;
  flag_t err;
  char file_in[1<<8], file_out[1<<8], file_err[1<<8];

  /* End of here doc limiter if the here doc bit in the `in` flag is set */
  char eod[1<<8];
  char here_string[1<<8];
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

void printf_tree(const struct node_t *const node, size_t level);
int run(const struct node_t *const node);
#endif
