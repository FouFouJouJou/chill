#include <unistd.h>
#include <job.h>
#include <parser.h>
#include <sys/types.h>
#include <sys/wait.h>

static size_t num = 0;
struct job_t *jobs[1<<8];

int schedule(struct job_t *job) {
  job->num = num++;
  jobs[job->num] = job;

  return (run(job->node));
}
