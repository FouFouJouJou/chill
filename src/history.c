#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <history.h>

struct history_t history;
char *history_file = "/tmp/chill_history";

static size_t read_from_file(const char *const file_name, char *buffer) {
  int fd, bytes_read;
  off_t raw_bytes;

  fd = open(file_name, O_CREAT|O_RDWR, 0644);
  if(fd == -1) {
    exit(80);
  }
  raw_bytes = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  bytes_read = read(fd, buffer, raw_bytes);
  if (bytes_read == -1) {
    exit(80);
  }
  if (bytes_read != raw_bytes) {
    perror("File not read\n");
  }
  buffer[bytes_read] = '\0';
  close(fd);
  return (size_t) bytes_read;
}

size_t read_history(struct history_t *const history) {
  size_t size, idx;
  char buffer[1<<8];
  char *buffer_p, *new_line_delim, *line;
  size = read_from_file(history_file, buffer);
  buffer_p = buffer;
  idx = 0;

  history->count = 0;
  history->start = history->cmds;
  history->finish = history->cmds;

  while ((new_line_delim = strchr(buffer_p, '\n'))) {
    int line_len;

    line_len = (int)(new_line_delim-buffer_p);
    line = calloc(line_len, sizeof(char));
    memcpy(line, buffer_p, line_len);
    line[line_len] = '\0';

    history->finish += 1;
    history->cmds[idx++] = line;
    buffer_p = new_line_delim+1;
  }

  history->count = idx;
  (void) size;
  return 0;
}

size_t append_cmd(const char *const cmd, struct history_t *const history) {
  /* NOTE: trying FILE* instead of fd for a more ANSI C compliant code */
  FILE *file = fopen(history_file, "a");
  fwrite(cmd, sizeof(char), strlen(cmd), file);
  fwrite("\n", sizeof(char), 1, file);
  fclose(file);

  read_history(history);

  return 1;
}

void printf_history(const struct history_t history) {
  char **start;
  printf("%ld\n", history.finish - history.start);
  for (start = history.start; start != history.finish; ++start) {
    printf("%s\n", *start);
  }
}
void free_history(const struct history_t history) {
  (void) history;
}
