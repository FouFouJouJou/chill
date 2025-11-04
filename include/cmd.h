#ifndef __CMD_H__
#define __CMD_H__
#include <stdint.h>
#define FLAG_OPTIONS_SIZE 2

typedef uint32_t flag_t;

struct cmd_t {
  char executable[1<<8];
  char *env[1<<8];
  char *argv[1<<8];
  size_t argc;
};

flag_t flag_to_options(flag_t flag);
flag_t flag_to_fd(flag_t flag);
void printf_cmd(const struct cmd_t *const cmd);
#endif
