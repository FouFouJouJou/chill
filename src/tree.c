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

  return EXIT_FAILURE;
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
/* TODO: here doc implementation */
static ssize_t run_redir_cmd(struct redir_node_t *const redir_node) {
  pid_t pid;
  flag_t in_options;
  flag_t out_options;
  flag_t err_options;

  int in_redir;
  int out_redir;
  int err_redir;
  int status;

  (void)pid;
  in_options = flag_to_options(redir_node->in);
  out_options = flag_to_options(redir_node->out);
  err_options = flag_to_options(redir_node->err);

  in_redir = in_options >> (FLAG_OPTIONS_SIZE-1) == 1;
  out_redir = out_options >> (FLAG_OPTIONS_SIZE-1) == 1;
  err_redir = err_options >> (FLAG_OPTIONS_SIZE-1) == 1;

  if (in_redir) {
    int fd;
    int here_doc;
    here_doc = (flag_to_options(redir_node->in) & 1);
    if (!here_doc) {
      fd = open(redir_node->file_in, O_CREAT|O_RDONLY, 0644);
#ifdef DEBUG
      printf("in <- %d\n", fd);
#endif
      if (fd == -1) {
	return errno;
      }
      redir_node->in |= fd;
    }
  }

  if (out_redir) {
    int fd;
    int append;
    append = (flag_to_options(redir_node->out) & 1);
    fd = open(redir_node->file_out, O_CREAT|O_WRONLY|(append ? O_APPEND : O_TRUNC), 0644);
#ifdef DEBUG
    printf("out -> %d\n", fd);
#endif
    if (fd == -1) {
      return errno;
    }
    redir_node->out |= fd;
  }

  if (err_redir) {
    int fd;
    int append;
    append = (flag_to_options(redir_node->err) & 1);
    fd = open(redir_node->file_err, O_CREAT|O_WRONLY|(append ? O_APPEND : O_TRUNC), 0644);
#ifdef DEBUG
    printf("err -> %d\n", fd);
#endif
    if (fd == -1) {
      return errno;
    }
    redir_node->err |= fd;
  }

  pid = fork();

  if (pid == 0) {
    if (in_redir) {
      dup2(flag_to_fd(redir_node->in), STDIN_FILENO);
    }
    if (out_redir) {
      dup2(flag_to_fd(redir_node->out), STDOUT_FILENO);
    }
    if (err_redir) {
      dup2(flag_to_fd(redir_node->err), STDERR_FILENO);
    }
    exit(run(redir_node->node));
  }

  waitpid(pid, &status, 0);

  if (in_redir) {
    close(flag_to_fd(redir_node->in));
  }
  if (out_redir) {
    close(flag_to_fd(redir_node->out));
  }
  if (err_redir) {
    close(flag_to_fd(redir_node->err));
  }

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }

  return EXIT_FAILURE;
}

static ssize_t run_pipe_cmd(struct double_node_t *const pipe_node) {
  pid_t pid_1, pid_2;
  int fds[2];
  int status_1;
  int status_2;

  if (pipe(fds) == -1) {
    return errno;
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

  close(fds[0]);
  close(fds[1]);

  waitpid(pid_1, &status_1, 0);
  waitpid(pid_2, &status_2, 0);


  if (WIFEXITED(status_1)) {
    int exit_code;
    exit_code = WEXITSTATUS(status_1);
    if (exit_code != EXIT_SUCCESS) {
      return exit_code;
    }
  }

  if (WIFEXITED(status_2)) {
    return WEXITSTATUS(status_2);
  }

  return EXIT_FAILURE;
}

ssize_t run(const struct node_t *const node) {
  switch(node->type) {
  case NODE_TYPE_CMD:
    return run_cmd((struct cmd_node_t *)node->node);
  case NODE_TYPE_AND:
    return run_and_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_OR:
    return run_or_cmd((struct double_node_t*)node->node);
  case NODE_TYPE_REDIR:
    return run_redir_cmd((struct redir_node_t *)node->node);
  case NODE_TYPE_PIPE:
    return run_pipe_cmd((struct double_node_t *)node->node);
  default:
    exit(80);
  }
}
