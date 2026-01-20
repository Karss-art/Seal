#include "shell.h"
#include <ctype.h>

char *trim(char *str) {
  char *end;

  /* Trim leading space */
  while (isspace((unsigned char)*str))
    str++;

  if (*str == 0)
    return str;

  /* Trim trailing space */
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  /* Write new null terminator */
  end[1] = '\0';

  return str;
}

void print_error(const char *msg) { fprintf(stderr, "seal: %s\n", msg); }
