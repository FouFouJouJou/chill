#ifndef __BUILT_IN__
#define __BUILT_IN__


char *which_(const char *const cmd, const char **env);
typedef int(*builtin_t)(size_t argc, char **argv, char **env);
builtin_t cmd_to_builtin(const char *const cmd);

#endif
