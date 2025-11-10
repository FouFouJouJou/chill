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

static char *pair_key(const char *const pair) {
  char *equals;
  char *result;
  int result_len;
  equals = strchr(pair, '=');
  assert(equals != NULL && equals != pair);
  result_len = equals-pair;

  result = calloc(result_len, sizeof(char));
  strncpy(result, pair, result_len);
  result[result_len] = '\0';

  return result;
}

static char *pair_value(const char *const pair) {
  char *equals;
  char *value;
  char *result;
  int value_size;
  equals = strchr(pair, '=');
  assert(equals != NULL);

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

char* replace(char* string, const char* substr, const char* new_str) {
  char* result;
  int i, j, count;
  int old_len, new_len;
  old_len = strlen(substr);
  new_len = strlen(new_str);

  for (i = 0; string[i] != '\0'; i++) {
    if (strstr(&string[i], substr) == &string[i]) {
      count+=1;
      i+=old_len-1;
    }
  }

  result = (char*)malloc(i + count * (new_len - old_len) + 1);
  if (result == NULL) {
    perror("memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  i=0;
  j=0;
  while (string[i] != '\0') {
    if (strstr(&string[i], substr) == &string[i]) {
      strcpy(&result[j], new_str);
      j+=new_len;
      i+=old_len;
    } else {
      result[j++] = string[i++];
    }
  }
  result[j] = '\0';
  return result;
}

int var_len(char *const string) {
  char *string_p;
  char *var_name;
  string_p = string;

  assert(*string_p == '$');
  var_name = string_p+1;

  if (*var_name == '\0' || (!isalnum(*var_name) && *var_name != '$' && *var_name != '_')) {
    return -1;
  }

  if (*var_name == '$' || isdigit(*var_name)) {
    return 2;
  }

  while (isalnum(*var_name) || *var_name == '_') {
    var_name += 1;
  }

  return var_name - string_p;
}

int extract_vars(char *string, char *vars[]) {
  char *string_p;
  char *var;
  int total;
  int string_len;
  (void) vars;
  string_p = string;
  string_len = strlen(string);
  total = 0;

  while ((var = strchr(string_p, '$')) != NULL || string_p > string+string_len) {
    int var_l;
    char *v;
    var_l = var_len(var);
    if (var_l == -1) {
      printf("var: $\n");
      string_p = var+1;
      continue;
    }
    v = calloc(var_l, sizeof(char));
    strncpy(v, var, var_l);
    v[var_l] = '\0';
    vars[total++] = v;
    string_p = var+var_l;
  }

  return total;
}

void evaluate(int argc, char **const argv, char **env) {
  char *var;
  char *env_var;
  char *arg;
  char *old_arg;
  char *eval_arg;
  char *vars[1<<8];
  int total_vars;
  int i;
  (void) argc;
  (void) argv;
  (void) env;

  arg = "$1 I love $FouFou/$JouJou";
  eval_arg = NULL;
  env_var = "$_";
  var = "a=value";
  printf("arg: %s\n", arg);
  printf("%s=%s\n", pair_key(var), pair_value(var));
  printf("%s(%d)\n", env_var, var_len(env_var));
  total_vars = extract_vars(arg, vars);
  for (i=0; i< total_vars; ++i) {
    if (eval_arg != NULL) {
      old_arg = eval_arg;
      eval_arg = replace(eval_arg, vars[i], "XXXX");
      free(old_arg);
    } else {
      eval_arg = replace(arg, vars[i], "XXXX");
    }
  }
}
