#ifndef __ENV_H__
#define __ENV_H__

#include <cmd.h>

#define MAX_ENV_CAP (1<<8)

struct environ_t {
  char *env[1<<8];
  size_t total;
};

void init_environ();
void printf_environ();
void free_environ();

size_t setup_env(struct cmd_t *cmd);
void printf_process_env(const char **const env);

char *getenv_(const char *key, char **env);
char *getenviron(const char *key);
size_t setenviron(const char *name, const char *value, int overwrite);
size_t setenvironstr(const char *pair);
size_t unset(const char *key);
size_t unsetstr(const char *pair);
/* int setenv_str(const char *pair); */

char* replace(char* string, const char* substr, const char* new_str);
void evaluate(int argc, char **const argv, char **env);

#endif
