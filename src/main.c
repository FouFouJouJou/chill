#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <parser.h>
#include <lex.h>
#include <env.h>
#include <cmd.h>
#include <builtin.h>
#include <prompt.h>
#include <history.h>
#include <job.h>

extern int exit_code;

void handle_sig(int sig) {
  printf("SIG: %d\n", sig);
}

int main() {
  char string[1<<8];
  signal(SIGCHLD, handle_sig);
  printf("%d\n", (int) getpid());

  init_environ();
  read_history();

  while (1) {
    struct job_t job;
    prompt(string);
    if (strlen(string) == 0) {
      continue;
    }

    job.node = parse(string);
#ifdef DEBUG
    printf_tree(job.node, 0);
#endif
    append_cmd(string);
    exit_code = schedule(&job);

    if (job.node->detached) {
      printf("[%ld] %ld\n", job.num, job.pid);
    }
    memset(string, 0, sizeof(string));
    /* free_tree(job.node); */
  }
  return EXIT_SUCCESS;
}
