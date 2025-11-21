#include <unistd.h>
#include <job.h>
#include <parser.h>
#include <sys/types.h>
#include <sys/wait.h>

static size_t num = 0;

int schedule(struct job_t *job) {
  int status;
  pid_t pid;
  job->num = num++;

  pid = fork();
  if (pid == 0) {
    exit(run(job->node));
  }
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return EXIT_FAILURE;
}
