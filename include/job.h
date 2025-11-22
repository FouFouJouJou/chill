#ifndef __JOB_H__
#define __JOB_H__

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <tree.h>
#define MAX_JOB_CAP (1<<4)

struct free_node_t {
  size_t num;
  struct free_node_t *next;
};

struct free_list_t {
  struct free_node_t *head;
  struct free_node_t *tail;
  size_t total;
};

struct job_t {
  struct node_t *node;
  size_t num;
  pid_t pid;
  uint8_t state;
};

void init_job_thread();
void init_free_list();
size_t get_free();
int schedule(struct node_t *node);

#endif
