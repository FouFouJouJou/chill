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
struct job_list_t free_list;
struct job_list_t alloc_list;

size_t recent = 0;
size_t total = 0;

char *job_state_to_string(enum job_state_t state) {
  switch(state) {
  case JOB_STATE_RUNNING:
    return "JOB_STATE_RUNNING";
  case JOB_STATE_STOPPED:
    return "JOB_STATE_STOPPED";
  case JOB_STATE_KILLED:
    return "JOB_STATE_KILLED";
  case JOB_STATE_TOTAL:
    return "JOB_STATE_TOTA";
  default:
    assert(0 && "JOB STATE UNKNOWN");
    exit(81);
  }
}

static struct job_node_t *add_node__(struct job_node_t *node, struct job_list_t *list) {
  if (list->head == NULL) {
    list->total += 1;
    list->head = node;
    return node;
  }

  else if (list->tail == NULL) {
    list->total += 1;
    list->tail = node;
    list->head->next = list->tail;
    return node;
  }

  list->tail->next = node;
  list->tail = node;

  list->total += 1;
  return node;
}

static struct job_node_t *add_node_(size_t num, struct job_list_t *list) {
  struct job_node_t *node;
  node = calloc(1, sizeof(struct job_node_t));
  node->num = num;
  node->next = NULL;

  node = add_node__(node, list);
  return node;
}

static struct job_node_t *get_free_(struct job_list_t *list) {
  struct job_node_t *node = list->head;
  assert(node != NULL && "LIST EMPTY");

  list->head = node->next;
  list->total -= 1;
  node->next = NULL;

  return node;
}

size_t lease_num(void) {
  struct job_node_t *node = get_free_(&free_list);
  add_node__(node, &alloc_list);
  return node->num;
}

struct job_node_t *get_num_(size_t num, struct job_list_t *list) {
  struct job_node_t *node;
  if (list->head->num == num) {
    node = list->head;
    list->head = list->head->next;
    node->next = NULL;
    return list->head;
  }

  node = list->head;

  while (node != NULL) {
    if (node->next == NULL) {
      break;
    }

    if (node->next->num == num) {
      return node;
    }

    node = node->next;
  }

  return NULL;
}

void free_num(size_t num) {
  struct job_node_t *node;
  struct job_node_t *previous_node = get_num_(num, &alloc_list);
  if (previous_node != alloc_list.head) {
    assert(previous_node != NULL);
  }
  if (previous_node == alloc_list.head) {
    alloc_list.head = alloc_list.head->next;
    node = previous_node;
    node->next = NULL;
    add_node__(node, &free_list);
  } else {
    node = previous_node->next;
    previous_node->next = previous_node->next->next;
  }

  add_node__(node, &free_list);
  printf("lease: %ld, free: %ld\n", alloc_list.total, free_list.total);
}

void init_free_list() {
  size_t i;
  free_list.head = NULL;
  free_list.tail = NULL;
  free_list.total = 0;

  alloc_list.head = NULL;
  alloc_list.tail = NULL;
  alloc_list.total = 0;

  for (i=0; i<MAX_JOB_CAP; ++i) {
    add_node_(i, &free_list);
  }
}

/* TODO: check for signal status */
void *schedule_(void *arg) {
  int status;
  pid_t pid;
  struct job_node_t *node;

  while (1) {
    node = alloc_list.head;
    while (node != NULL) {
      pid = waitpid(jobs[node->num].pid, &status, WNOHANG);
      if (pid > 0) {
	printf("[%d] done\n", pid);
	/* free_num(jobs[node->num].num); */
      }
    }
  }
  (void) arg;
  pthread_exit(0);
}

void init_job_thread() {
  pthread_t th;
  pthread_create(&th, 0, schedule_, NULL);
}

struct job_t *create_job(pid_t pid) {
  struct job_t *job = calloc(1, sizeof(struct job_t));
  job->pid = pid;
  job->state =JOB_STATE_RUNNING;
  job->num = lease_num();
  return job;
}

void printf_list(struct job_list_t *list) {
  struct job_node_t *node = list->head;
  while (node != NULL) {
    printf("%ld ", node->num);
    node = node->next;
  }
  printf("\n");
}

static void printf_job(struct job_t job) {
  printf("[%ld] %s %d\n", job.num+1, job_state_to_string(job.state), job.pid);
}

void printf_jobs() {
  struct job_node_t *node;
  printf("total jobs: %ld\n", alloc_list.total);
  for (node = alloc_list.head; node != NULL; node = node->next) {
    /* printf("job\n"); */
    /* printf("%p\n", (void *)(node)); */
    printf_job(jobs[node->num]);
  }
}

int schedule(struct node_t *node) {
  pid_t pid;
  int status, ret;
  struct job_t *job;
  node = process(node);
  ret = 0;

  if (node == NULL) {
    return 1;
  }

  if (node->detached) {
    pid = fork();

    if (pid == 0) {
      exit(run(node));
    }

    job = create_job(pid);
    jobs[job->num] = *job;
    free(job);

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
