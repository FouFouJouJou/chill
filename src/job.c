#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <job.h>
#include <parser.h>
#include <tree.h>

static size_t num = 0;
struct job_t *jobs[1<<8];

static void *schedule_(void *arg) {
  int status;
  struct job_t *job = (struct job_t *)arg;
  run(job->node);
  waitpid(job->pid, &status, 0);
  printf("[%ld] done\n", job->num, job->pid);
  free_tree(job->node);
  free(job);
  pthread_exit(0);
}

int schedule(struct node_t *node) {
  pthread_t th;
  struct job_t *job = calloc(1, sizeof(struct job_t));
  job->num = num++;
  job->node = node;

  jobs[job->num] = job;
  pthread_create(&th, NULL, schedule_, (void *)job);
  return 0;
}
