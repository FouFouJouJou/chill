#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

ssize_t run_pipe_cmd(const struct double_node_t *const pipe_node) {
  pid_t pid_1, pid_2;
  int status_1, status_2;
  int fds[2];

  if (pipe(fds) < 0) {
    return 80;
  }

  pid_1 = fork();
  if (pid_1 == 0) {
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    exit(run(pipe_node->left_node));
  }

  pid_2 = fork();
  if (pid_2 == 0) {
    close(fds[1]);
    dup2(fds[0], STDIN_FILENO);
    exit(run(pipe_node->right_node));
  }

  waitpid(pid_1, &status_1, 0);
  waitpid(pid_2, &status_2, 0);

  return 0;
}

ssize_t run(const struct node_t *const node) {
  switch(node->type) {
  case NODE_TYPE_OR:
    return run_or_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_AND:
    return run_and_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_PIPE:
    return run_pipe_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_CMD:
    return run_cmd((struct cmd_node_t *)node->node);
  default:
    exit(80);
  }
}
