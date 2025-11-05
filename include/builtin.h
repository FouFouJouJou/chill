#ifndef __BUILT_IN__
#define __BUILT_IN__

typedef int(*builtin_t)(int argc, char **argv, char **env);

builtin_t cmd_to_builtin(const char *const cmd);

#endif
