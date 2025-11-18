#ifndef __HISTORY_H__
#define __HISTORY_H__

#define MAX_HISTORY_CAP (1<<8)

/* using a circular buffer */
struct history_t {
  char *cmds[MAX_HISTORY_CAP];
  char **start;
  char **finish;
};

struct history_t init_history();
size_t read_history(struct history_t *const history);
size_t append_cmd(const char *const cmd, struct history_t *const history);
void printf_history(const struct history_t history);
void free_history(const struct history_t history);

#endif
