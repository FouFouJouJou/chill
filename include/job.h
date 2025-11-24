#ifndef __JOB_H__
#define __JOB_H__

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <tree.h>
#define MAX_JOB_CAP (1<<4)

enum job_state_t {
  JOB_STATE_RUNNING = 0x0,
  JOB_STATE_STOPPED,
  JOB_STATE_KILLED,
  JOB_STATE_DONE,
  JOB_STATE_TOTAL
};

struct job_node_t {
  size_t num;
  struct job_node_t *next;
};

struct job_list_t {
  struct job_node_t *head;
  struct job_node_t *tail;
  size_t total;
};

struct job_t {
  struct node_t *node;
  enum job_state_t state;
  size_t num;
  pid_t pid;
};

void init_job_thread();
void init_free_list();
struct job_t *register_job(pid_t pid);
int schedule(struct node_t *node);
void free_job(pid_t pid);
void printf_jobs();

#endif
