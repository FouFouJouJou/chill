#ifndef __ENV_H__
#define __ENV_H__

void printf_env(void);
size_t setup_env(char **cmd_env);
void printf_process_env(char **const env);
void evaluate(int argc, char **const argv, char **env);
#endif
