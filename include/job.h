#ifndef __JOB_H__
#define __JOB_H__

#include <stdlib.h>
#include <tree.h>

struct job_t {
  struct node_t *node;
  size_t num;
};

int schedule(struct job_t *job);

#endif
