#include "shell.h"

static int is_redir_operator(const char *token) {
  return (strcmp(token, "<") == 0 || strcmp(token, ">") == 0 ||
          strcmp(token, ">>") == 0 || strcmp(token, "2>") == 0 ||
          strcmp(token, "2>&1") == 0);
}

static RedirType get_redir_type(const char *token) {
  if (strcmp(token, "<") == 0)
    return REDIR_IN;
  if (strcmp(token, ">") == 0)
    return REDIR_OUT;
  if (strcmp(token, ">>") == 0)
    return REDIR_APPEND;
  if (strcmp(token, "2>") == 0)
    return REDIR_ERR;
  if (strcmp(token, "2>&1") == 0)
    return REDIR_ERR_OUT;
  return REDIR_NONE;
}

Pipeline *parse_pipeline(char **tokens, int token_count) {
  if (token_count == 0)
    return NULL;

  Pipeline *pipeline = malloc(sizeof(Pipeline));
  if (!pipeline) {
    perror("malloc");
    return NULL;
  }

  /* Count number of commands (separated by |) */
  int cmd_count = 1;
  for (int i = 0; i < token_count; i++) {
    if (strcmp(tokens[i], "|") == 0) {
      cmd_count++;
    }
  }

  pipeline->commands = calloc(cmd_count, sizeof(Command));
  pipeline->cmd_count = cmd_count;

  int cmd_idx = 0;
  int arg_start = 0;
  int background = 0;

  for (int i = 0; i <= token_count; i++) {
    /* Process command at pipe or end */
    if (i == token_count || strcmp(tokens[i], "|") == 0) {
      Command *cmd = &pipeline->commands[cmd_idx];

      /* Count arguments and redirections */
      int argc = 0;
      int redir_count = 0;

      for (int j = arg_start; j < i; j++) {
        if (strcmp(tokens[j], "&") == 0) {
          background = 1;
        } else if (is_redir_operator(tokens[j])) {
          redir_count++;
          j++; /* Skip filename (except for 2>&1) */
          if (strcmp(tokens[j - 1], "2>&1") != 0) {
            if (j >= i) {
              print_error("syntax error: missing filename");
              free_pipeline(pipeline);
              return NULL;
            }
          }
        } else {
          argc++;
        }
      }

      /* Allocate argv */
      cmd->argv = calloc(argc + 1, sizeof(char *));
      cmd->argc = argc;

      /* Allocate redirections */
      if (redir_count > 0) {
        cmd->redirs = calloc(redir_count, sizeof(Redirection));
        cmd->redir_count = redir_count;
      }

      /* Fill argv and redirections */
      int arg_idx = 0;
      int redir_idx = 0;

      for (int j = arg_start; j < i; j++) {
        if (strcmp(tokens[j], "&") == 0) {
          /* Skip & */
        } else if (is_redir_operator(tokens[j])) {
          cmd->redirs[redir_idx].type = get_redir_type(tokens[j]);
          if (strcmp(tokens[j], "2>&1") != 0) {
            j++;
            cmd->redirs[redir_idx].filename = strdup(tokens[j]);
          } else {
            cmd->redirs[redir_idx].filename = NULL;
          }
          redir_idx++;
        } else {
          cmd->argv[arg_idx++] = strdup(tokens[j]);
        }
      }

      cmd->argv[argc] = NULL;
      cmd->background = background;

      cmd_idx++;
      arg_start = i + 1;
    }
  }

  return pipeline;
}

void free_pipeline(Pipeline *pipeline) {
  if (!pipeline)
    return;

  for (int i = 0; i < pipeline->cmd_count; i++) {
    Command *cmd = &pipeline->commands[i];

    /* Free argv */
    if (cmd->argv) {
      for (int j = 0; j < cmd->argc; j++) {
        if (cmd->argv[j]) {
          free(cmd->argv[j]);
        }
      }
      free(cmd->argv);
    }

    /* Free redirections */
    if (cmd->redirs) {
      for (int j = 0; j < cmd->redir_count; j++) {
        if (cmd->redirs[j].filename) {
          free(cmd->redirs[j].filename);
        }
      }
      free(cmd->redirs);
    }
  }

  free(pipeline->commands);
  free(pipeline);
}
