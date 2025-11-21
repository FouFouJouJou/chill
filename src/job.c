#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <job.h>
#include <parser.h>
#include <tree.h>

static size_t num = 1;
struct job_t *jobs[1<<8];
size_t recent = 1;

static void *schedule_(void *arg) {
  int status;
  struct job_t *job = (struct job_t *)arg;
  waitpid(job->pid, &status, 0);
  printf("[%ld] %d done\n", job->num, job->pid);
  free_tree(job->node);
  free(job);
  pthread_exit(0);
}

int schedule(struct node_t *node) {
  pthread_t th;
  pid_t pid;
  struct job_t *job = calloc(1, sizeof(struct job_t));
  recent = num;
  job->num = num++;
  job->node = node;
  job->state = 0;
  jobs[job->num] = job;

  pid = fork();
  setpgid(pid, pid);
  if (pid == 0) {
    exit(run(job->node));
  }

  job->pid = pid;
  printf("[%ld] %d\n", job->num, job->pid);
  jobs[job->num] = job;
  pthread_create(&th, NULL, schedule_, (void *)job);
  return 0;
}
