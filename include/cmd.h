#ifndef __CMD_H__
#define __CMD_H__

struct cmd_t {
  char executable[1<<8];
  char *env[1<<8];
  char *argv[1<<8];
  size_t argc;
};

void printf_cmd(const struct cmd_t *const cmd);
#endif
