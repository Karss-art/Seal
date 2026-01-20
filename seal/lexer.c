#include "shell.h"

static int is_special_char(char c) {
  return (c == '|' || c == '&' || c == '<' || c == '>' || c == ' ' ||
          c == '\t' || c == '\n');
}

char **tokenize(const char *line, int *token_count) {
  char **tokens = malloc(sizeof(char *) * MAX_TOKENS);
  if (!tokens) {
    perror("malloc");
    return NULL;
  }

  int count = 0;
  const char *p = line;
  char buffer[MAX_LINE];
  int buf_idx = 0;
  int in_quotes = 0;
  char quote_char = 0;

  while (*p) {
    /* Skip leading whitespace */
    while (*p == ' ' || *p == '\t') {
      p++;
    }

    if (*p == '\0')
      break;

    buf_idx = 0;
    in_quotes = 0;

    while (*p && (in_quotes || !is_special_char(*p) ||
                  (*p != ' ' && *p != '\t' && *p != '\n'))) {
      /* Handle quotes */
      if ((*p == '"' || *p == '\'') && !in_quotes) {
        in_quotes = 1;
        quote_char = *p;
        p++;
        continue;
      } else if (*p == quote_char && in_quotes) {
        in_quotes = 0;
        p++;
        continue;
      }

      /* Handle escape */
      if (*p == '\\' && *(p + 1)) {
        p++;
        buffer[buf_idx++] = *p++;
        continue;
      }

      /* Check for special operators */
      if (!in_quotes) {
        /* Handle >> */
        if (*p == '>' && *(p + 1) == '>') {
          if (buf_idx > 0) {
            buffer[buf_idx] = '\0';
            tokens[count++] = strdup(buffer);
            buf_idx = 0;
          }
          tokens[count++] = strdup(">>");
          p += 2;
          break;
        }
        /* Handle 2> */
        else if (*p == '2' && *(p + 1) == '>') {
          if (buf_idx > 0) {
            buffer[buf_idx] = '\0';
            tokens[count++] = strdup(buffer);
            buf_idx = 0;
          }
          /* Check for 2>&1 */
          if (*(p + 2) == '&' && *(p + 3) == '1') {
            tokens[count++] = strdup("2>&1");
            p += 4;
          } else {
            tokens[count++] = strdup("2>");
            p += 2;
          }
          break;
        }
        /* Handle single character operators */
        else if (*p == '|' || *p == '&' || *p == '<' || *p == '>') {
          if (buf_idx > 0) {
            buffer[buf_idx] = '\0';
            tokens[count++] = strdup(buffer);
            buf_idx = 0;
          }
          buffer[0] = *p;
          buffer[1] = '\0';
          tokens[count++] = strdup(buffer);
          p++;
          break;
        }
        /* Break on whitespace */
        else if (*p == ' ' || *p == '\t' || *p == '\n') {
          break;
        }
      }

      buffer[buf_idx++] = *p++;
    }

    if (buf_idx > 0) {
      buffer[buf_idx] = '\0';
      tokens[count++] = strdup(buffer);
    }

    if (count >= MAX_TOKENS - 1) {
      break;
    }
  }

  tokens[count] = NULL;
  *token_count = count;

  return tokens;
}

void free_tokens(char **tokens, int count) {
  if (!tokens)
    return;

  for (int i = 0; i < count; i++) {
    if (tokens[i]) {
      free(tokens[i]);
    }
  }
  free(tokens);
}
