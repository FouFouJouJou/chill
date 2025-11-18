#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#include <builtin.h>
#include <history.h>

/* TODO: implement which to locate executables */
extern struct history_t history;

static char *get_path(char **env) {
  char **env_p;
  for (env_p = env; *env_p != NULL; ++env_p) {
    if (strstr(*env_p, "PATH") == *env_p) {
      return *env_p;
    }
  }
  return NULL;
}

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
    printf("File '%s' exists and is executable.\n", filepath);
    return 1;
  }
  return errno;
}

char *which_(const char *const cmd, const char **env) {
  char *env_path;
  char *path;
  if (!is_executable(cmd)) {
    return NULL;
  }
  env_path = get_path((char **)env);
  assert(env_path != NULL);
  path = which__(cmd, env_path);
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
  assert(argc == 2);
  if (chdir(argv[1]) == -1) {
    return errno;
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
  printf_history(history);
  return 0;
}

static int exit_(size_t argc, char **argv, char **env) {
  (void) argc;
  (void) argv;
  (void) env;

  free_history(&history);
  exit(0);
}

builtin_t cmd_to_builtin(const char *const cmd) {
  builtin_t fn = NULL;
  if (!strncmp(cmd, "history", 7)) {
    fn = history_;
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
  if (!strncmp(cmd, "cd", 2)) {
    fn = cd;
  }

  return fn;
}
