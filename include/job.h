#ifndef __JOB_H__
#define __JOB_H__

#include <stdlib.h>
#include <tree.h>

struct job_t {
  struct node_t *node;
  size_t num, pid;
};

int schedule(struct node_t *node);

#endif
