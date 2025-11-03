#include <stdio.h>
#include <stdlib.h>
#include <cmd.h>
#include <tree.h>

int main() {
  struct cmd_node_t cmd_node_1;
  struct cmd_node_t cmd_node_2;
  struct cmd_node_t cmd_node_3;

  struct double_node_t and_node;
  struct double_node_t pipe_node;

  struct node_t node_1;
  struct node_t node_2;
  struct node_t node_3;
  struct node_t node_4;
  int status;

  struct cmd_t cmd_1 = {
    .executable = "/bin/ls",
    .env = { NULL },
    .argv = { "ls", "-l", "/Users/foufou/", NULL },
    .argc = 2
  };

  struct cmd_t cmd_2 = {
    .executable = "/bin/cat",
    .env = { NULL },
    .argv = { "/bin/cat", "/Users/foufou/.vimrc", NULL },
    .argc = 2
  };

  struct cmd_t cmd_3 = {
    .executable = "/usr/bin/wc",
    .env = { NULL },
    .argv = { "wc", "-l", NULL },
    .argc = 2
  };

  cmd_node_1.cmd = &cmd_1;
  node_1.type = NODE_TYPE_CMD;
  node_1.node = (void*)&cmd_node_1;

  cmd_node_2.cmd = &cmd_2;
  node_2.type = NODE_TYPE_CMD;
  node_2.node = (void*)&cmd_node_2;

  cmd_node_3.cmd = &cmd_3;
  node_3.type = NODE_TYPE_CMD;
  node_3.node = (void*)&cmd_node_3;

  and_node.left_node = &node_1;
  and_node.right_node = &node_2;

  node_3.type = NODE_TYPE_OR;
  node_3.node = (void*)&and_node;

  /* status = run(&node_3); */
  /* printf("exit code: %d\n", status); */

  pipe_node.left_node = &node_1;
  pipe_node.right_node = &node_3;

  node_4.type = NODE_TYPE_PIPE;
  node_4.node = (void*)&pipe_node;

  status = run(&node_4);
  printf("exit code: %d\n", status);

  exit(EXIT_SUCCESS);
}
