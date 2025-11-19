#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <env.h>
#include <cmd.h>
#include <ctype.h>

extern char **environ;
struct environ_t environ_;
extern int exit_code;

void init_environ() {
  char **env;
  for (env = environ; *env != NULL; ++env) {
    size_t len;
    int idx;
    idx = (int)(env-environ);
    len = strlen(*env);
    environ_.env[idx] = calloc(len+1, sizeof(char));
    memcpy(environ_.env[idx], *env, len);
    environ_.env[idx][len] = '\0';
  }
  environ_.total = env-environ;
  environ_.env[environ_.total] = NULL;
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

static size_t env_total(char **env) {
  size_t total;
  char **env_p;
  total = 0;
  for (env_p = env; *env_p != NULL; ++total, ++env_p) {}
  return total;
}

static char *pair_value(const char *const pair) {
  char *equals;
  char *value;
  char *result;
  int value_size;
  equals = strchr(pair, '=');
  if (equals == NULL) {
    return NULL;
  }

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

static char *findenv_(const char *key, char **env) {
  char **env_p;
  for (env_p = env; *env_p != NULL; ++env_p) {
    if (strstr(*env_p, key) == *env_p) {
      return *env_p;
    }
  }
  return NULL;
}

char *getenv_(const char *key, char **env) {
  char *var;
  var = findenv_(key, env);
  if (var == NULL) {
    fprintf(stderr, "chill: %s not set\n", key);
    return NULL;
  }
  return pair_value(var);
}

static size_t putenv_(const char *name, const char *value, char **env) {
  char *var;
  size_t name_len, value_len, var_len;
  name_len = strlen(name);
  value_len = strlen(value);
  var_len = name_len + 1 + value_len + 1;

  var = findenv_(name, env);
  assert(var != NULL);

  var = realloc(var, var_len*sizeof(char));
  memcpy(strchr(var, '=')+1, value, value_len);
  var[var_len] = '\0';
  return 0;
}

static size_t addenv_(const char *name, const char *value, char **env) {
  size_t name_len, value_len, var_len, total;
  total = env_total(env);

  name_len = strlen(name);
  value_len = strlen(value);
  var_len = name_len+1+value_len+1;
  env[total] = calloc(var_len, sizeof(char));
  strcat(env[total], name);
  env[total][name_len] = '=';
  strcat(env[total]+name_len+1, value);
  env[total][var_len] = '\0';
  env[total+1] = NULL;

  return 1;
}

static size_t setenv_(const char *name, const char *value, int overwrite, char **env) {
  char *val;
  size_t total;
  total = env_total(env);

  if (total+1 > MAX_ENV_CAP) {
    errno = ENOMEM;
    return -1;
  }

  if (name == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (overwrite == 0) {
    return total;
  }

  val = findenv_(name, env);
  if (val == NULL) {
    return addenv_(name, value, env);
  } else {
    return putenv_(name, value, env);
  }
}

static size_t setenvstr_(const char *pair, char **env) {

  if (strchr(pair, '=') == NULL) {
    return setenv_(pair, "", 1, env);
  }
  else {
    char *key, *val;
    size_t result;
    key = pair_key(pair);
    val = pair_value(pair);

    result = setenv_(key, val, 1, env);

    free(key);
    free(val);

    return result;
  }

  return 0;
}

size_t setenvstr(const char *pair) {
  environ_.total += setenvstr_(pair, environ_.env);
  return environ_.total;
}

char *getenv(const char *name) {
  return getenv_(name, environ_.env);
}

size_t putenv(const char *name, const char *value) {
  return putenv_(name, value, environ_.env);
}

size_t setenv(const char *name, const char *value, int overwrite) {
  environ_.total += setenv_(name, value, overwrite, environ_.env);
  return environ_.total;
}

void printf_environ() {
  size_t i;
  for (i=0; i<environ_.total; ++i) {
    printf("%ld %s\n", i+1, environ_.env[i]);
  }
}

void free_environ() {
  size_t i;
  for (i=0; i<environ_.total; ++i) {
    free(environ_.env[i]);
  }
}

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

size_t setup_env(struct cmd_t *cmd) {
  char **var;

  for (var = environ_.env; *var != NULL; var++) {
    if (strstr(*var, "PATH") == *var
	|| strstr(*var, "HOME") == *var
	|| strstr(*var, "USER") == *var
	|| strstr(*var, "hello") == *var
	) {
      char *env_var;
      size_t var_len;
      var_len = strlen(*var);
      env_var = calloc(var_len+1, sizeof(char));
      memcpy(env_var, *var, var_len);
      env_var[var_len] = '\0';
      cmd->env[cmd->total_env++] = env_var;
    }
  }

  cmd->env[cmd->total_env] = NULL;

  return cmd->total_env;
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
