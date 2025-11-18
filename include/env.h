#ifndef __ENV_H__
#define __ENV_H__

#include <cmd.h>

struct environ_t {
  char *env[1<<8];
  size_t total;
};

void init_environ();
void free_environ();
size_t setup_env(struct cmd_t *cmd);
void printf_process_env(const char **const env);
char *getenv_(char *key, char **env);
int setenv(const char *name, const char *value, int overwrite);
char* replace(char* string, const char* substr, const char* new_str);
void evaluate(int argc, char **const argv, char **env);

#endif
