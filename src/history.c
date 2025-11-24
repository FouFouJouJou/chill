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

  fd = open(file_name, O_CREAT|O_RDONLY, 0644);
  if(fd == -1) {
    perror("file open error");
    exit(80);
  }
  raw_bytes = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  bytes_read = read(fd, buffer, raw_bytes);
  if (bytes_read == -1) {
    perror("reading bytes error");
    exit(80);
  }
  if (bytes_read != raw_bytes) {
    perror("File not read\n");
  }
  buffer[bytes_read] = '\0';
  close(fd);
  return (size_t) bytes_read;
}

size_t sync_history() {
  size_t idx;
  char buffer[1<<8];
  char *buffer_p, *new_line_delim, *line;
  read_from_file(history_file, buffer);
  buffer_p = buffer;
  idx = 0;

  memset(history.cmds, 0, MAX_HISTORY_CAP*sizeof(char *));
  history.count = 0;

  while ((new_line_delim = strchr(buffer_p, '\n'))) {
    int line_len;
    line_len = (int)(new_line_delim-buffer_p)+1;
    line = calloc(line_len, sizeof(char));
    memcpy(line, buffer_p, line_len-1);
    line[line_len] = '\0';

    history.cmds[idx++] = line;
    buffer_p = new_line_delim+1;
  }

  history.count = idx;
  return 0;
}

size_t append_cmd(const char *const cmd) {
  /* NOTE: trying FILE* instead of fd for a more ANSI C compliant code */
  FILE *file = fopen(history_file, "a");
  fwrite(cmd, sizeof(char), strlen(cmd), file);
  fwrite("\n", sizeof(char), 1, file);
  fclose(file);

  sync_history();

  return 1;
}

void printf_history() {
  size_t i;
  for (i=0; i<history.count; ++i) {
    printf("%ld %s\n", i+1, history.cmds[i]);
  }
}

void clear_history() {
  FILE *file;
  file = fopen(history_file, "w");
  fclose(file);
}

void free_history() {
  size_t i;
  for (i=0; i<history.count; ++i) {
    free(history.cmds[i]);
  }
}
