#ifndef __ENV_H__
#define __ENV_H__

size_t setup_env(char **cmd_env);
void printf_process_env(const char **const env);
char* replace(char* string, const char* substr, const char* new_str);
void evaluate(int argc, char **const argv, char **env);

#endif
