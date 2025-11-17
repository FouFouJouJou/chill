#ifndef __HISTORY_H__
#define __HISTORY_H__

/* using a circular buffer */
struct history_t {
  char *cmds[1<<8];
  char *start;
  char *finish;
};

size_t read_history(struct history_t *history);

#endif
