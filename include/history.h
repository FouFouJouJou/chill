#ifndef __HISTORY_H__
#define __HISTORY_H__

#define MAX_HISTORY_CAP (1<<8)

/* using a circular buffer */
/* TODO: "wc <<< I love the way you lie" error fix for history */

struct history_t {
  char *cmds[MAX_HISTORY_CAP];
  size_t count;
  /* TODO: for circular buffer impl later */
  char **start;
  char **finish;
};

size_t sync_history(void);
size_t append_cmd(const char *const cmd);
void printf_history(void);
void clear_history(void);
void free_history(void);

#endif
