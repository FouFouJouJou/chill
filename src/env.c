#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <env.h>
#include <ctype.h>

extern char **environ;
extern int exit_code;

void printf_process_env(const char **const env) {
  const char **var;
  printf("[");
  for (var = env; *var != NULL; var++) {
    printf("%s", *var);
    if (*(var+1) != NULL) {
      printf(", ");
    }
  }
  printf("], ");
}

size_t setup_env(char **cmd_env) {
  size_t env_size;
  char **var;
  env_size = 0;

  for (var = cmd_env; *var != NULL; var++) {
    env_size++;
  }

  for (var = environ; *var != NULL; var++) {
    if (strstr(*var, "PATH") == *var || strstr(*var, "HOME") == *var || strstr(*var, "USER") == *var) {
      cmd_env[env_size++] = *var;
    }
  }

  cmd_env[env_size] = NULL;

  return env_size;
}

char *pair_key(const char *const pair) {
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
  count = 0;

  for (i = 0; string[i] != '\0'; i++) {
    if (strstr(&string[i], substr) == &string[i]) {
      count+=1;
      i+=old_len-1;
    }
  }

  result = (char*)calloc(i + count * abs(new_len - old_len) + 1, sizeof(char));
  if (result == NULL) {
    fprintf(stderr, "%d %d\n", new_len, old_len);
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

  if (*var_name == '\0' || (!isalnum(*var_name) && *var_name != '$' && *var_name != '_' && *var_name != '?')) {
    return -1;
  }

  if (*var_name == '$' || isdigit(*var_name) || *var_name == '?') {
    return 2;
  }

  while (isalnum(*var_name) || *var_name == '_') {
    var_name += 1;
  }

  return var_name - string_p;
}

/* returns list of variables in `string`, each element is in the `$var_name` format */
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
      /* printf("var: $\n"); */
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

static char *evaluate_special_env_value(char *key, char **env) {
  char *value;
  (void) env;
  if (!strncmp(key, "$", 1)) {
    value = calloc(3, sizeof(char));
    sprintf(value, "%d", getpid());
    return value;
  }

  if (!strncmp(key, "?", 1)) {
    value = calloc(3, sizeof(char));
    sprintf(value, "%d", exit_code);
    return value;
  }

  return NULL;
}

char *evaluate_env_value(char *key, int argc, char **argv, char **env) {
  char **env_var;
  char *value;
  (void) argc;
  (void) argv;

  if ((value = evaluate_special_env_value(key, env)) != NULL) {
    return value;
  }

  for (env_var = env; *env_var != NULL; ++env_var) {
    if (!strncmp(key, *env_var, strlen(key))) {
      value = pair_value(*env_var);
      return value;
    }
  }

  return NULL;
}

/* TODO: fix echo "hello world $name" bug */
void evaluate(int argc, char **const argv, char **env) {
  char *arg;
  char *old_arg;
  char *eval_arg;
  char *vars[1<<8];
  int total_vars;
  int i, k;

  for (i = 0; i< argc; ++i) {
    eval_arg = NULL;
    old_arg = NULL;
    arg = argv[i];
    total_vars = extract_vars(arg, vars);
    for (k = 0; k< total_vars; ++k) {
      char *env_val;
      char *subs;
      env_val = evaluate_env_value(vars[k]+1, argc, argv, env);
      subs = env_val == NULL ? "" : env_val;
      if (k == 0) {
	eval_arg = replace(arg, vars[k], subs);
      }
      else {
	old_arg = eval_arg;
	eval_arg = replace(eval_arg, vars[k], subs);
	free(old_arg);
      }
    }

    if (eval_arg == NULL) {
      argv[i] = arg;
    }
    else {
      memcpy(argv[i], eval_arg, strlen(eval_arg));
      argv[i][strlen(eval_arg)] = '\0';
      free(eval_arg);
    }
  }
}
