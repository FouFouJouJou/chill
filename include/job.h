#ifndef __JOB_H__
#define __JOB_H__

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <tree.h>

struct job_t {
  struct node_t *node;
  size_t num;
  pid_t pid;
  uint8_t state;
};

int schedule(struct node_t *node);

#endif
