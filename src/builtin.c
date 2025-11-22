#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#include <builtin.h>
#include <env.h>
#include <history.h>
#include <job.h>
#include <signal.h>

extern size_t recent;

/* TODO: create an api without exposing jobs */
extern struct job_t jobs[MAX_JOB_CAP];

int find_file_in_directory(const char *dirname, const char *filename_to_find) {
  DIR *dir;
  struct dirent *entry;

  dir = opendir(dirname);
  if (dir == NULL) {
#ifdef DEBUG
    perror("Error opening directory");
#endif
    return -1;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, filename_to_find) == 0) {
#ifdef DEBUG
      fprintf(stderr, "File '%s' found in directory '%s'.\n", filename_to_find, dirname);
#endif
      closedir(dir);
      return 1;
    }
  }

#ifdef DEBUG
  printf("File '%s' not found in directory '%s'.\n", filename_to_find, dirname);
#endif
  closedir(dir);
  return 0;
}

/* TODO: only returns one path match of the executable, make it a list of paths as return*/
static char *which__(const char *const cmd, const char *path) {
  char *delim;
  char dir[1<<8];
  if(path == NULL) {
    return NULL;
  }

  while ((delim = strchr(path, ':')) != NULL) {
    int dir_size;
    dir_size = (delim-path);
    memcpy(dir, path, dir_size);
    dir[dir_size] = '\0';
    if (find_file_in_directory(dir, cmd) == 1) {
      char *result;
      int path_size;
      path_size = dir_size + strlen(cmd) + 1;
      result = calloc(path_size, sizeof(char));
      memcpy(result, dir, dir_size);
      result[dir_size] = '/';
      memcpy(result+dir_size+1, cmd, strlen(cmd));
      result[path_size] = '\0';
      return result;
    }
    path = delim+1;
  }
  return NULL;
}

static int is_executable(const char *const filepath) {
  if (access(filepath, F_OK | X_OK) == 0) {
#ifdef DEBUG
    printf("File '%s' exists and is executable.\n", filepath);
#endif
    return 1;
  }
  return errno;
}

char *which_(const char *const cmd, const char **env) {
  char *env_path;
  char *path;
  path = NULL;

  if (!is_executable(cmd)) {
    return NULL;
  }

  if (strstr(cmd, "./") == cmd) {
    size_t cwd_len, cmd_len, path_len;
    char cwd[1<<8];
    cmd_len = strlen(cmd);

    if (cmd_len == 2) {
      return NULL;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
      perror("chill: getcwd() error");
      return NULL;
    }

    cwd_len = strlen(cwd);
    path_len = cwd_len+cmd_len-1;
    path = calloc(path_len+1, sizeof(char));
    memcpy(path, cwd, cwd_len);
    memcpy(path+cwd_len, cmd+1, cmd_len-1);
    path[path_len] = '\0';
  } else {
    env_path = getenv_("PATH", (char **)env);
    path = which__(cmd, env_path);
  }

  return path;
}

/* TODO: only uses one argument, extend it to any n argument */
static int which(size_t argc, char **argv, char **env) {
  char *path;
  (void) argc;

  path = which_(argv[1], (const char **)env);
  if (path == NULL) {
    return 1;
  }

  printf("%s\n", path);
  return 0;
}

static int cd(size_t argc, char **argv, char **env) {
  (void) env;
  if (argc == 2 && chdir(argv[1]) == -1) {
    return errno;
  }

  if (argc == 1) {
    char *home = getenv_("HOME", env);
    if (chdir(home) == -1) {
      free(home);
      return errno;
    }
  }

  return 0;
}

static int echo(size_t argc, char **argv, char **env) {
  size_t i;
  (void) env;
  for (i=0; i<argc-1; ++i) {
    printf("%s", (argv+1)[i]);
    if (i+1 != argc) {
      printf(" ");
    }
  }
  printf("\n");
  return 0;
}

static int history_(size_t argc, char **argv, char **env) {
  (void) argc;
  (void) argv;
  (void) env;
  if (argc == 2 && !strncmp(argv[1], "-c", strlen(argv[1]))) {
    clear_history();
    return 0;
  }
  printf_history();
  return 0;
}

static int export_b(size_t argc, char **argv, char **env) {
  size_t i;
  (void) env;
  for (i=1; i<argc; ++i) {
    setenvironstr(argv[i]);
  }

  return 0;
}

static int unset_(size_t argc, char **argv, char **env) {
  size_t i;
  (void) env;
  for (i=1; i<argc; ++i) {
    unset(argv[i]);
  }

  return 0;
}

static int fg(size_t argc, char **argv, char **env) {
  (void) argc;
  (void) argv;
  (void) env;

  printf("resuming %d\n", jobs[recent].pid);
  tcsetpgrp(STDIN_FILENO, jobs[recent].pid);
  kill(-jobs[recent].pid, SIGCONT);
  return 0;
}

static int job(size_t argc, char **argv, char **env) {
  (void) argc;
  (void) argv;
  (void) env;

  return 0;
}

static int exit_(size_t argc, char **argv, char **env) {
  (void) argc;
  (void) argv;
  (void) env;

  free_environ();
  free_history();
  exit(0);
}

builtin_t cmd_to_builtin(const char *const cmd) {
  builtin_t fn = NULL;
  if (!strncmp(cmd, "history", 7)) {
    fn = history_;
  }
  if (!strncmp(cmd, "unset", 6)) {
    fn = unset_;
  }
  if (!strncmp(cmd, "export", 6)) {
    fn = export_b;
  }
  if (!strncmp(cmd, "which", 5)) {
    fn = which;
  }
  if (!strncmp(cmd, "echo", 4)) {
    fn = echo;
  }
  if (!strncmp(cmd, "exit", 4)) {
    fn = exit_;
  }
  if (!strncmp(cmd, "job", 3)) {
    fn = job;
  }
  if (!strncmp(cmd, "fg", 2)) {
    fn = fg;
  }
  if (!strncmp(cmd, "cd", 2)) {
    fn = cd;
  }

  return fn;
}
