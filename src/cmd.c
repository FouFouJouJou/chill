#include <stdio.h>
#include <cmd.h>

void printf_cmd(const struct cmd_t *const cmd) {
  printf("exec: %s\n", cmd->executable);
}
