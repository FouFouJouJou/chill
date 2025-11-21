#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <prompt.h>

size_t prompt(char *prompt) {
  char line[1<<8];
  int size;

  printf("> ");
  fgets(line, sizeof(line), stdin);
  size = strcspn(line, "\n");
  line[size] = '\0';

  memcpy(prompt, line, size);
  prompt[size] = '\0';

  return size;
}
