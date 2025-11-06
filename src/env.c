#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <env.h>
#include <ctype.h>

extern char **environ;

void printf_env(void) {
  char **var;
  for (var = environ; *var != NULL; var++) {
    printf("%s\n", *var);
  }
}

void printf_process_env(char **const env) {
  char **var;
  for (var = env; *var != NULL; var++) {
    printf("%s\n", *var);
  }
}

size_t setup_env(char **cmd_env) {
  size_t env_size;
  char **var;
  env_size = 0;

  for (var = cmd_env; *var != NULL; var++) {
    env_size++;
  }

  for (var = environ; *var != NULL; var++) {
    cmd_env[env_size++] = *var;
  }

  cmd_env[env_size] = NULL;

  return env_size;
}

static char *get_env_value(const char *const var) {
  (void) var;
  return NULL;
}

static char *evaluate_arg(char *const arg, char **env) {
  (void) arg;
  (void) env;
  (void) get_env_value;
  return NULL;
}

void evaluate(int argc, char **const argv, char **env) {
  int i;
  printf("argv: ");
  for (i = 0; i< argc-1; ++i) {
    char *eval;
    eval = evaluate_arg((argv+1)[i], env);
    printf("%s ", eval);
    if (eval != NULL) {
      free(eval);
    }
  }

  printf("\n");
}
