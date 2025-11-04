#include <stdio.h>
#include <cmd.h>

flag_t flag_to_options(flag_t flag) {
  return flag >> (sizeof(flag_t)*8-FLAG_OPTIONS_SIZE);
}

flag_t flag_to_fd(flag_t flag) {
  flag_t options_max;
  flag_t mask;
  options_max = (1<<FLAG_OPTIONS_SIZE)-1;
  mask = options_max<<((sizeof(flag_t)*8)-FLAG_OPTIONS_SIZE);
  return flag & ~mask;
}

void printf_cmd(const struct cmd_t *const cmd) {
  printf("exec: %s\n", cmd->executable);
}
