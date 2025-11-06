#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmd.h>
#include <tree.h>

int main() {
  struct cmd_node_t cmd_node_1;
  struct cmd_node_t cmd_node_2;

  struct double_node_t and_node;
  struct redir_node_t redir_node;
  struct double_node_t pipe_node;

  struct node_t node_1;
  struct node_t node_2;
  struct node_t node_3;
  struct node_t node_4;
  struct node_t node_5;

  int status;

  struct cmd_t cmd_1 = {
    "/bin/ls",
    { "ls", "/Users/foufou/", "/Users/foufou/.vimrc", "$foufou2", NULL },
    4,
    { "foufou=FouFou1", NULL },
  };

  struct cmd_t cmd_2 = {
    "/usr/bin/wc",
    { "wc", "/Users/foufou/.vimrc", NULL },
    1,
    { "foufou=foufou._.", NULL },
  };

  cmd_node_1.cmd = &cmd_1;
  node_1.type = NODE_TYPE_CMD;
  node_1.node = (void*)&cmd_node_1;

  cmd_node_2.cmd = &cmd_2;
  node_2.type = NODE_TYPE_CMD;
  node_2.node = (void*)&cmd_node_2;

  redir_node.in = 0xC0000000;
  strcpy(redir_node.eod, "EOD");
  strcpy(redir_node.here_string, "I love the way you lie");
  redir_node.out = 0x00000000;
  redir_node.err = 0x00000000;

  strcpy(redir_node.file_in, "/tmp/fileXXXXXX.txt");
  strcpy(redir_node.file_out, "/tmp/out.txt");
  strcpy(redir_node.file_err, "/tmp/err.txt");

  node_3.type = NODE_TYPE_REDIR;
  node_3.node = (void*)&redir_node;

  and_node.left_node = &node_1;
  and_node.right_node = &node_2;

  node_4.type = NODE_TYPE_AND;
  node_4.node = (void*)&and_node;
  (void)node_4;

  pipe_node.left_node = &node_1;
  pipe_node.right_node = &node_2;

  node_5.type = NODE_TYPE_PIPE;
  node_5.node = (void*)&pipe_node;

  redir_node.node = &node_2;

  (void)node_5;
  status = run(&node_1);
  exit(status);

  return 0;
}
