#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <env.h>
#include <cmd.h>

flag_t set_input_options(flag_t *flag, size_t options) {
  assert(options < (1<<INPUT_FLAG_OPTIONS_SIZE));
  *flag |= (options << (sizeof(flag_t)*8-INPUT_FLAG_OPTIONS_SIZE));
  return *flag;
}

flag_t set_options(flag_t *flag, size_t options) {
  assert(options < (1<<FLAG_OPTIONS_SIZE));
  *flag |= (options << (sizeof(flag_t)*8-FLAG_OPTIONS_SIZE));
  return *flag;
}

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

flag_t input_flag_to_options(flag_t flag) {
  return flag >> (sizeof(flag_t)*8-INPUT_FLAG_OPTIONS_SIZE);
}

flag_t input_flag_to_fd(flag_t flag) {
  flag_t options_max;
  flag_t mask;
  options_max = (1<<INPUT_FLAG_OPTIONS_SIZE)-1;
  mask = options_max<<((sizeof(flag_t)*8)-INPUT_FLAG_OPTIONS_SIZE);
  return flag & ~mask;
}

void printf_cmd(const struct cmd_t *const cmd) {
  size_t i;
  printf("CMD(exec: %s, ", cmd->executable);
  printf("env: ");
  if (cmd->env[0] == NULL) {
    printf("<empty>, ");
  } else {
    printf_process_env((const char **const)cmd->env);
  }

  printf(", args: ");
  if (cmd->argc == 1) {
    printf("<empty>, ");
  } else {
    printf("[");
    for (i=0; i<cmd->argc; ++i) {
      printf("%s", cmd->argv[i]);
      if (i+1 != cmd->argc) {
	printf(", ");
      }
    }
    printf("])\n");
  }
}
