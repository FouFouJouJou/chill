#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <job.h>
#include <parser.h>
#include <tree.h>

struct job_t *jobs[MAX_JOB_CAP];
struct job_list_t free_list;
struct job_list_t alloc_list;

size_t recent = 0;
size_t total = 0;

static pthread_mutex_t job_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t alloc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t free_mutex = PTHREAD_MUTEX_INITIALIZER;

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

static size_t lease_num(void) {
  struct job_node_t *node;
  pthread_mutex_lock(&free_mutex);
  node = get_free_(&free_list);
  pthread_mutex_unlock(&free_mutex);

  pthread_mutex_lock(&alloc_mutex);
  add_node__(node, &alloc_list);
  pthread_mutex_unlock(&alloc_mutex);
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
  struct job_node_t *previous_node;

  pthread_mutex_lock(&alloc_mutex);
  previous_node = get_num_(num, &alloc_list);
  if (previous_node != alloc_list.head) {
    if (previous_node == NULL) {
      pthread_mutex_unlock(&alloc_mutex);
      exit(88);
    }
  }
  if (previous_node == alloc_list.head) {
    alloc_list.head = alloc_list.head->next;
    node = previous_node;
    node->next = NULL;
    pthread_mutex_lock(&free_mutex);
    add_node__(node, &free_list);
    pthread_mutex_unlock(&free_mutex);
  } else {
    node = previous_node->next;
    previous_node->next = previous_node->next->next;
  }
  pthread_mutex_unlock(&alloc_mutex);

  pthread_mutex_lock(&free_mutex);
  add_node__(node, &free_list);
  pthread_mutex_lock(&alloc_mutex);
  printf("lease: %ld, free: %ld\n", alloc_list.total, free_list.total);
  pthread_mutex_unlock(&alloc_mutex);
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

void free_job(pid_t pid) {
  (void) pid;
}

/* TODO: check for signal status */
void *schedule_(void *arg) {
  int status;
  pid_t pid;
  struct job_node_t *node;

  while (1) {
    pthread_mutex_lock(&alloc_mutex);
    node = alloc_list.head;
    if (node == NULL) {
      pthread_mutex_unlock(&alloc_mutex);
      continue;
    }

    while (node != NULL) {
      pthread_mutex_unlock(&alloc_mutex);
      pthread_mutex_lock(&job_mutex);
      if (jobs[node->num]->state == JOB_STATE_STOPPED) {
	/* TODO: schedule remove of the job */
	pthread_mutex_unlock(&job_mutex);
	continue;
      }
      pid = waitpid(jobs[node->num]->pid, &status, WNOHANG);
      pthread_mutex_unlock(&job_mutex);
      if (pid > 0) {
	printf("[%d] done\n", pid);
	pthread_mutex_lock(&job_mutex);
	jobs[node->num]->state = JOB_STATE_STOPPED;
	pthread_mutex_unlock(&job_mutex);
	/* free_job(pid); */
	/* free(jobs[node->num]); */
	/* free_num(jobs[node->num]->num); */
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

static struct job_t *create_job(pid_t pid) {
  struct job_t *job = calloc(1, sizeof(struct job_t));
  job->pid = pid;
  job->state = JOB_STATE_RUNNING;
  job->num = lease_num();
  return job;
}

struct job_t *register_job(pid_t pid) {
  struct job_t *job = create_job(pid);
  pthread_mutex_lock(&job_mutex);
  jobs[job->num] = job;
  pthread_mutex_unlock(&job_mutex);
  return job;
}

static void printf_job(struct job_t *job) {
  printf("[%ld] %s %d\n", job->num+1, job_state_to_string(job->state), job->pid);
}

void printf_jobs() {
  struct job_node_t *node;
  printf("total jobs: %ld\n", alloc_list.total);
  pthread_mutex_lock(&job_mutex);
  for (node = alloc_list.head; node != NULL; node = node->next) {
    /* printf("job\n"); */
    /* printf("%p\n", (void *)(node)); */
    printf_job(jobs[node->num]);
  }
  pthread_mutex_unlock(&job_mutex);
}

int schedule(struct node_t *node) {
  pid_t pid;
  int status, ret;
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

    register_job(pid);

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
