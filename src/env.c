#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <env.h>
#include <ctype.h>

extern char **environ;

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

/* static char *pair_key(const char *const var) { */
/* } */

static char *pair_value(const char *const pair) {
  char *equals;
  char *value;
  char *result;
  int value_size;
  equals = strchr(pair, '=');
  assert(equals);

  value = equals+1;
  if (*value == '\0') {
    result = calloc(1, sizeof(char));
    result[0] = '\0';
    return result;
  }
  value_size = pair+strlen(pair)-equals;
  result = calloc(value_size, sizeof(char));
  strncpy(result, value, value_size);
  result[value_size] = '\0';
  return result;
}

static char* replace(char* string, const char* substr, const char* new_str) {
  char* result;
  int i, j, count = 0;
  int old_len = strlen(substr);
  int new_len = strlen(new_str);

  for (i = 0; string[i] != '\0'; i++) {
    if (strstr(&string[i], substr) == &string[i]) {
      count++;
      i += old_len - 1;
    }
  }

  result = (char*)malloc(i + count * (new_len - old_len) + 1);
  if (result == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  i = 0;
  j = 0;
  while (string[i] != '\0') {
    if (strstr(&string[i], substr) == &string[i]) {
      strcpy(&result[j], new_str);
      j += new_len;
      i += old_len;
    } else {
      result[j++] = string[i++];
    }
  }
  result[j] = '\0';
  return result;
}

static char *evaluate_arg(char *const arg, char **env) {
  (void) arg;
  (void) env;
  (void) pair_value;
  return NULL;
}

void evaluate(int argc, char **const argv, char **env) {
  char *value;
  char *input;
  (void) argc;
  (void) argv;
  (void) env;
  (void) evaluate_arg;
  (void) replace;

  input = "$key This is the test function with a $key in the middle";
  value = pair_value("key=I love the way you lie");
  argv[0] = NULL;
  printf("%s(%ld)\n", value, strlen(value));
  printf("input(pre): %s\n", input);
  printf("intput (pre): %s\n", replace(input, "$key", "I love it"));
  /* int i; */
  /* printf("argv: "); */
  /* for (i = 0; i< argc-1; ++i) { */
  /*   char *eval; */
  /*   eval = evaluate_arg((argv+1)[i], env); */
  /*   printf("%s ", eval); */
  /*   if (eval != NULL) { */
  /*     free(eval); */
  /*   } */
  /* } */

  /* printf("\n"); */
}
