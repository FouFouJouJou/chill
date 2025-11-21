#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <parser.h>
#include <lex.h>
#include <env.h>
#include <cmd.h>
#include <builtin.h>
#include <prompt.h>
#include <history.h>
#include <job.h>

extern int exit_code;

/* void handle_suspend(int sig) { */
/*   (void) sig; */
/*   printf("SIG\n"); */
/*   exit(0); */
/* } */

int main() {
  char string[1<<8];
  /* signal(SIGINT, handle_suspend); */
  printf("%d\n", (int) getpid());

  init_environ();
  read_history();

  while (1) {
    struct job_t job;
    prompt(string);

    job.node = parse(string);
#ifdef DEBUG
    printf_tree(job.node, 0);
#endif
    append_cmd(string);
    exit_code = schedule(&job);
    memset(string, 0, sizeof(string));
    free_tree(job.node);
  }
  return EXIT_SUCCESS;
}
