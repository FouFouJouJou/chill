#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <job.h>
#include <parser.h>
#include <tree.h>

struct job_t jobs[MAX_JOB_CAP];
struct free_list_t free_list;
size_t recent = 0;
size_t total = 0;

struct free_node_t *add_free_node_(size_t num, struct free_list_t *free_list) {
  struct free_node_t *node;
  node = calloc(1, sizeof(struct free_node_t));
  node->num = num;
  node->next = NULL;

  if (free_list->head == NULL) {
    free_list->head = node;
    return node;
  }

  else if (free_list->tail == NULL) {
    free_list->head->next = node;
    free_list->tail = node;
    return node;
  }

  free_list->tail->next = node;
  free_list->tail = node;
  return node;
}

struct free_node_t *add_free_node(size_t num) {
  return add_free_node_(num, &free_list);
}

static size_t get_free_(struct free_list_t *list) {
  size_t result;
  struct free_node_t *node = list->head;
  assert(node != NULL && "MAX REACHED");
  result = node->num;
  list->head = node->next;
  free(node);
  list->total-=1;

  return result;
}

size_t get_free() {
  return get_free_(&free_list);
}

void init_free_list() {
  size_t i;
  free_list.head = NULL;
  free_list.tail = NULL;
  free_list.total = 0;
  for (i=0; i<MAX_JOB_CAP; ++i) {
    add_free_node(i);
    free_list.total++;
  }
}

void *schedule_(void *arg) {
  int status;
  pid_t pid;
  (void) arg;
  while (1) {
    pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
      /* printf("%d done\n", pid); */
    }
  }
  pthread_exit(0);
}

void init_job_thread() {
  pthread_t th;
  pthread_create(&th, 0, schedule_, NULL);
}

int schedule(struct node_t *node) {
  pid_t pid;
  int status, ret;
  node = process(node);

  if (node == NULL) {
    return 1;
  }

  if (node->detached) {
    pid = fork();

    if (pid == 0) {
      exit(run(node));
    }

    waitpid(pid, &status, WNOHANG);

    if (WIFEXITED(status)) {
      ret = WEXITSTATUS(status);
    }
  }
  else {
    ret = run(node);
  }

  return ret;
}
