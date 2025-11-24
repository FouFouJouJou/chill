#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <builtin.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <tree.h>
#include <cmd.h>
#include <env.h>

int exit_code;
static const char *const here_doc_file_name = "/tmp/fileXXXXXX.txt";
static const char *const here_string_file_name = "/tmp/fileXXXXXXX.txt";

char *node_type_to_string(enum node_type_t type) {
  switch (type) {
  case NODE_TYPE_CMD:
    return "NODE_TYPE_CMD";
  case NODE_TYPE_AND:
    return "NODE_TYPE_AND";
  case NODE_TYPE_OR:
    return "NODE_TYPE_OR";
  case NODE_TYPE_REDIR:
    return "NODE_TYPE_REDIR";
  case NODE_TYPE_PIPE:
    return "NODE_TYPE_PIPE";

  default:
    assert(0 && "UNREACHABLE");
  }
}

/* TODO: make `file_name` random */
static int read_here_string(const char *const here_string) {
  int fd;
  fd = open(here_string_file_name, O_CREAT|O_RDWR|O_TRUNC, 0644);
  if (fd == -1) {
    perror("Error opening input file");
    exit(errno);
  }

  write(fd, here_string, strlen(here_string));
  write(fd, "\n", 1);

  lseek(fd, 0, SEEK_SET);

  return fd;
}

/* TODO: make `file_name` random */
static int read_here_doc(const char *const eod) {
  int fd;
  char buffer[1<<8] = {0};
  char line[1<<8] = {0};
  size_t input_size;
  size_t bits_written;
  size_t eod_len;
  eod_len = strlen(eod);
  fd = open(here_doc_file_name, O_CREAT|O_RDWR|O_TRUNC, 0644);
  if (fd == -1) {
    fprintf(stderr, "Error opening input file\n");
    exit(errno);
  }

  input_size = 0;
  while(1) {
    size_t line_len;
    printf(">> ");
    fgets(line, sizeof(line), stdin);
    line_len = strlen(line);
    if (line_len >= eod_len && !strncmp(line+line_len-eod_len-1, eod, eod_len)) {
      strncpy(buffer+input_size, line, line_len-eod_len-1);
      input_size+=line_len-eod_len-1;
      break;
    }
    strncpy(buffer+input_size, line, strlen(line));
    input_size+=strlen(line);
  }

  buffer[input_size] = '\0';
#ifdef DEBUG
  printf("%s\n", buffer);
#endif
  bits_written = write(fd, buffer, strlen(buffer));
  if (bits_written != input_size) {
    perror("tmp file write failure");
    exit(80);
  }

  lseek(fd, 0, SEEK_SET);

  return fd;
}

static int run_cmd(const struct cmd_node_t *cmd_node) {
  int status;
  pid_t pid;

  pid = fork();
  if (pid == 0) {
    struct cmd_t *cmd = cmd_node->cmd;
    int ret = execve(cmd->executable, cmd->argv, cmd->env);
    if (ret == -1) {
      fprintf(stderr, "run_cmd: error\n");
      exit(errno);
    }
  }

  waitpid(pid, &status, 0);

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }

  return EXIT_FAILURE;
}

static int run_and_cmd(const struct node_t *and_node) {
  int status1, status2;

  status1 = run(and_node->left_node);

  if (status1 != EXIT_SUCCESS) {
    return status1;
  }

  status2 = run(and_node->right_node);
  return status2;
}

static int run_or_cmd(const struct node_t *or_node) {
  int status1, status2;

  status1 = run(or_node->left_node);

  if (status1 == EXIT_SUCCESS) {
    return status1;
  }

  status2 = run(or_node->right_node);
  return status2;
}

/* TODO: use masking to set fd in the flag and not `|` */
/* TODO: delete the input file after running if `here_string` or `here_doc` */
static int run_redir_cmd(struct redir_node_t *const redir_node) {
  pid_t pid;
  flag_t in_options, out_options, err_options;

  int in_redir, out_redir, err_redir;
  int status;

  in_options = input_flag_to_options(redir_node->in);
  out_options = flag_to_options(redir_node->out);
  err_options = flag_to_options(redir_node->err);

  in_redir = in_options >> (INPUT_FLAG_OPTIONS_SIZE-1) == 1;
  out_redir = out_options >> (FLAG_OPTIONS_SIZE-1) == 1;
  err_redir = err_options >> (FLAG_OPTIONS_SIZE-1) == 1;

  if (in_redir) {
    int fd;
    int here_doc;
    int here_string;
    here_doc = (in_options & 1);
    here_string = (in_options >> 1 & 1);
    if (here_doc) {
      fd = read_here_doc(redir_node->eod);
    }
    else if(here_string) {
      fd = read_here_string(redir_node->here_string);
    }
    else {
      fd = open(redir_node->file_in, O_RDONLY, 0644);
      if (fd == -1) {
	return errno;
      }
    }
    redir_node->in |= fd;
#ifdef DEBUG
    printf("in <- %d\n", input_flag_to_fd(redir_node->in));
#endif
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
      if(dup2(input_flag_to_fd(redir_node->in), STDIN_FILENO) == -1) {
	perror("dup2 failed");
	exit(errno);
      }
    }
    if (out_redir) {
      if (dup2(flag_to_fd(redir_node->out), STDOUT_FILENO) == -1) {
	perror("dup2 failed");
	exit(errno);
      }
    }
    if (err_redir) {
      if (dup2(flag_to_fd(redir_node->err), STDERR_FILENO) == -1) {
	perror("dup2 failed");
	exit(errno);
      }
    }
    exit(run(redir_node->node));
  }

  waitpid(pid, &status, 0);

  if (in_redir) {
    close(input_flag_to_fd(redir_node->in));
    if (in_options == REDIR_IN_FLAG_HERE_STRING) {
      remove(here_string_file_name);
    }
    else if (in_options == REDIR_IN_FLAG_HERE_DOC) {
      remove(here_doc_file_name);
    }
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

static int run_pipe_cmd(const struct node_t *pipe_node) {
  pid_t pid_1, pid_2;
  int fds[2];
  int status_1;
  int status_2;

  if (pipe(fds) == -1) {
    return errno;
  }

  pid_1 = fork();
  if (pid_1 == -1) {
      perror("fork failed");
      exit(errno);
  }
  if (pid_1 == 0) {
    close(fds[0]);
    if (dup2(fds[1], STDOUT_FILENO) == -1) {
      perror("dup2 failed");
      exit(errno);
    }
    exit(run(pipe_node->left_node));
  }

  pid_2 = fork();
  if (pid_2 == 0) {
    close(fds[1]);
    if (dup2(fds[0], STDIN_FILENO)) {
      perror("dup2 failed");
      exit(errno);
    }
    exit(run(pipe_node->right_node));
  }

  close(fds[0]);
  close(fds[1]);

  waitpid(pid_1, &status_1, 0);
  waitpid(pid_2, &status_2, 0);


  if (WIFEXITED(status_1)) {
    int ret;
    ret = WEXITSTATUS(status_1);
    if (ret != EXIT_SUCCESS) {
      return exit_code;
    }
  }

  if (WIFEXITED(status_2)) {
    return WEXITSTATUS(status_2);
  }

  return EXIT_FAILURE;
}

struct node_t *process(struct node_t *node) {
  switch (node->type) {
  case NODE_TYPE_CMD: {
    char *executable_path;
    builtin_t fn;
    struct cmd_node_t *cmd_node;

    cmd_node = (struct cmd_node_t *)node->node;
    setup_env(cmd_node->cmd);
    evaluate(cmd_node->cmd->argc, cmd_node->cmd->argv, cmd_node->cmd->env);
    memcpy(cmd_node->cmd->executable, cmd_node->cmd->argv[0], strlen(cmd_node->cmd->argv[0]));
    cmd_node->cmd->argv[0][strlen(cmd_node->cmd->argv[0])] = '\0';
    fn = cmd_to_builtin(cmd_node->cmd->executable);
    if (fn != NULL) {
      node->builtin = 1;
      return node;
    }

    node->builtin = 0;

    executable_path=which_(cmd_node->cmd->executable, (const char **)cmd_node->cmd->env);
    if (executable_path == NULL) {
      fprintf(stderr, "chill: command not found: %s\n", cmd_node->cmd->executable);
      return NULL;
    }
    if (executable_path != cmd_node->cmd->executable) {
      memcpy(cmd_node->cmd->executable, executable_path, strlen(executable_path));
      cmd_node->cmd->executable[strlen(executable_path)] = '\0';
      free(executable_path);
    } else {
      return NULL;
    }

    return node;
  }
  case NODE_TYPE_AND:
  case NODE_TYPE_OR:
  case NODE_TYPE_PIPE:
    process(node->left_node);
    process(node->right_node);
    return node;
  case NODE_TYPE_REDIR:
    process(((struct redir_node_t *)node->node)->node);
    return node;
  default:
    assert(0 && "NODE UNKNOWN");
  }
}

int run(struct node_t *const node) {
  assert(node != NULL);
  switch (node->type) {
  case NODE_TYPE_CMD: {
    struct cmd_node_t *cmd_node;
    cmd_node = (struct cmd_node_t *)node->node;
    if (node->builtin == 1) {
      builtin_t fn;
      fn = cmd_to_builtin(cmd_node->cmd->executable);
      assert(fn != NULL);
      return fn(cmd_node->cmd->argc, cmd_node->cmd->argv, cmd_node->cmd->env);
    }

    return run_cmd(cmd_node);
  }
  case NODE_TYPE_AND:
    return run_and_cmd(node);
  case NODE_TYPE_OR:
    return run_or_cmd(node);
  case NODE_TYPE_REDIR:
    return run_redir_cmd((struct redir_node_t *)node->node);
  case NODE_TYPE_PIPE:
    return run_pipe_cmd(node);
  default:
    exit(80);
  }
}

int process_and_run(struct node_t *node) {
  node = process(node);
  return run(node);
}

static char *node_type_symbol_to_string(enum node_type_t type) {
  switch (type) {
  case NODE_TYPE_AND:
    return "&&";
  case NODE_TYPE_OR:
    return "||";
  case NODE_TYPE_PIPE:
    return "|";
  default:
    exit(80);
  }
}

void printf_redir(struct redir_node_t *redir_node) {
  flag_t in_options, out_options, err_options;
  int in_redir, out_redir, err_redir;

  in_options = input_flag_to_options(redir_node->in);
  out_options = flag_to_options(redir_node->out);
  err_options = flag_to_options(redir_node->err);

  in_redir = in_options >> (INPUT_FLAG_OPTIONS_SIZE-1) == 1;
  out_redir = out_options >> (FLAG_OPTIONS_SIZE-1) == 1;
  err_redir = err_options >> (FLAG_OPTIONS_SIZE-1) == 1;

  printf("REDIR: ");

  if (in_redir) {
    switch (in_options) {
    case REDIR_IN_FLAG_FILE:
      printf("(< %s)", redir_node->file_in);
      break;
    case REDIR_IN_FLAG_HERE_DOC:
      printf("(<< %s)", redir_node->eod);
      break;
    case REDIR_IN_FLAG_HERE_STRING:
      printf("(<<< %s)", redir_node->here_string);
      break;
    }
  }

  if (out_redir) {
    switch (out_options) {
    case REDIR_OUT_FLAG_APPEND:
      printf("(>> %s)", redir_node->file_out);
      break;
    case REDIR_OUT_FLAG_TRUNC:
      printf("(> %s)", redir_node->file_out);
      break;
    }
  }

  if (err_redir) {
    switch (err_options) {
    case REDIR_ERR_FLAG_APPEND:
      printf("(>> %s)", redir_node->file_err);
      break;
    case REDIR_ERR_FLAG_TRUNC:
      printf("(> %s)", redir_node->file_err);
      break;
    }
  }
  printf("\n");
}

void printf_tree(const struct node_t *const node, size_t level) {
  printf("%*c", (int)level, ' ');
  switch (node->type) {
  case NODE_TYPE_CMD: {
    struct cmd_node_t *cmd_node;
    cmd_node = (struct cmd_node_t *) node->node;
    printf_cmd(cmd_node->cmd);
    break;
  }
  case NODE_TYPE_REDIR: {
    struct redir_node_t *redir_node = (struct redir_node_t *)node->node;
    printf_redir(redir_node);
    printf_tree(redir_node->node, level+PRINTF_PADDING);
    break;
  }
  default: {
    printf("%s\n", node_type_symbol_to_string(node->type));
    printf_tree(node->left_node, level+PRINTF_PADDING);
    printf_tree(node->right_node, level+PRINTF_PADDING);
    break;
  }
  }
}

void free_tree(struct node_t *head) {
  switch (head->type) {
  case NODE_TYPE_REDIR: {
    struct redir_node_t *redir_node;
    redir_node = ((struct redir_node_t *)(head->node));
    free_tree(redir_node->node);
    break;
  }
  case NODE_TYPE_CMD: {
    struct cmd_node_t *cmd_node;
    cmd_node = (struct cmd_node_t *)(head->node);
    free_cmd(cmd_node->cmd);
    break;
  }
  default:
    free_tree(head->left_node);
    free_tree(head->right_node);
    break;
  }
  free(head);
}
