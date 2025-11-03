#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <tree.h>
#include <cmd.h>

static ssize_t run_cmd(const struct cmd_node_t *cmd_node) {
  int status, exit_code;
  pid_t pid;

  pid = fork();

  if (pid == 0) {
    struct cmd_t *cmd = cmd_node->cmd;
    exit_code = execve(cmd->executable, cmd->argv, cmd->env);
    if (exit_code == -1) {
      exit(errno);
    }
  }

  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return -1;
}

static ssize_t run_and_cmd(const struct double_node_t *and_node) {
  int status1, status2;

  status1 = run(and_node->left_node);

  if (status1 != 0) {
    return status1;
  }

  status2 = run(and_node->right_node);
  return status2;
}

static ssize_t run_or_cmd(const struct double_node_t *or_node) {
  int status1, status2;

  status1 = run(or_node->left_node);

  if (!status1) {
    return status1;
  }

  status2 = run(or_node->right_node);
  return status2;
}

/* TODO: use masking to set fd in the flag and not `|` */
static ssize_t run_redir_cmd(struct redir_node_t *const redir_node) {
  if (redir_node->in >> (sizeof(redir_node->in)*8-1)) {
    int fd = open(redir_node->file_in, O_CREAT);
    printf("%d\n", fd);
    redir_node->in |= fd;
    printf("%u\n", redir_node->in);
    /* close(fd); */
  }

  if (redir_node->out >> (sizeof(redir_node->out)*8-1)) {
    int fd = open(redir_node->file_out, O_CREAT);
    redir_node->out |= fd;
    /* close(fd); */
  }

  if (redir_node->err >> (sizeof(redir_node->err)*8-1)) {
    int fd = open(redir_node->file_err, O_CREAT);
    redir_node->err |= fd;
    /* close(fd); */
  }
  return 0;
}

ssize_t run(const struct node_t *const node) {
  switch(node->type) {
  case NODE_TYPE_REDIR:
    return run_redir_cmd((struct redir_node_t *)node->node);
  case NODE_TYPE_OR:
    return run_or_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_AND:
    return run_and_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_CMD:
    return run_cmd((struct cmd_node_t *)node->node);
  default:
    exit(80);
  }
}
