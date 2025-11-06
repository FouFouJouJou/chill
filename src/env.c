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
  char *equals;
  char *value;
  size_t var_len;
  size_t value_len;

  equals = strchr(var, '=');
  var_len = strlen(var);
  assert(equals != NULL);

  /* if (*value == '\0') { */
  /*   return space */
  /* } */

  /* Fou=val */

  value_len = var+var_len-equals+2;
  value = calloc(value_len, sizeof(char));
  strncpy(value, equals+1, value_len);
  value[value_len] = '\0';

  return value;
}

static char *evaluate_arg(char *const arg, char **env) {
  (void) env;
  /* if (var[0] == '\'') { */
  /*   return NULL; */
  /* } */

  /* if (!strncmp(var, "$", strlen(var))) { */
  /*   return NULL; */
  /* } */

  if (arg[0] == '$') {
    char *var_name;
    int idx;
    idx = 0;
    while(arg[idx+1] != '\0' && (isalpha(arg[idx+1]) || isdigit(arg[idx+1]))) {
      idx++;
    }
    var_name = calloc(idx+1, sizeof(char));
    strncpy(arg, arg+1, idx);
    var_name[idx] = '\0';
    return var_name;
  }

  return NULL;
}

void evaluate(int argc, char **const argv, char **env) {
  int i;
  (void) env;
  printf("argv: ");
  for (i = 0; i< argc-1; ++i) {
    char *eval;
    eval = evaluate_arg((argv+1)[i], env);
    printf("%s ", eval);
    free(eval);
  }

  printf("\n");
}
