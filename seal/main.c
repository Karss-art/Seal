#include "shell.h"

/* Global shell state */
ShellState g_shell;

int main(int argc, char *argv[]) {
  char line[MAX_LINE];
  char **tokens;
  int token_count;
  Pipeline *pipeline;

  /* Initialize shell */
  init_shell();

  /* Main REPL loop */
  while (1) {
    /* Print prompt */
    print_prompt();

    /* Read line */
    if (fgets(line, sizeof(line), stdin) == NULL) {
      if (feof(stdin)) {
        printf("\n");
        break;
      }
      continue;
    }

    /* Trim whitespace */
    char *trimmed = trim(line);

    /* Skip empty lines */
    if (strlen(trimmed) == 0) {
      continue;
    }

    /* Tokenize */
    tokens = tokenize(trimmed, &token_count);
    if (tokens == NULL || token_count == 0) {
      continue;
    }

    /* Check for built-in exit */
    if (strcmp(tokens[0], "exit") == 0) {
      free_tokens(tokens, token_count);
      break;
    }

    /* Parse pipeline */
    pipeline = parse_pipeline(tokens, token_count);
    if (pipeline == NULL) {
      print_error("parse error");
      free_tokens(tokens, token_count);
      continue;
    }

    /* Execute pipeline */
    execute_pipeline(pipeline);

    /* Cleanup */
    free_pipeline(pipeline);
    free_tokens(tokens, token_count);
  }

  /* Cleanup shell */
  cleanup_shell();

  return 0;
}

void init_shell(void) {
  /* Initialize shell state */
  memset(&g_shell, 0, sizeof(ShellState));

  /* Check if interactive */
  g_shell.shell_terminal = STDIN_FILENO;
  g_shell.is_interactive = isatty(g_shell.shell_terminal);

  if (g_shell.is_interactive) {
    /* Loop until we are in the foreground */
    while (tcgetpgrp(g_shell.shell_terminal) !=
           (g_shell.shell_pgid = getpgrp())) {
      kill(-g_shell.shell_pgid, SIGTTIN);
    }

    /* Ignore interactive and job-control signals */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    /* Put ourselves in our own process group */
    g_shell.shell_pgid = getpid();
    if (setpgid(g_shell.shell_pgid, g_shell.shell_pgid) < 0) {
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Grab control of the terminal */
    tcsetpgrp(g_shell.shell_terminal, g_shell.shell_pgid);

    /* Save default terminal attributes for shell */
    tcgetattr(g_shell.shell_terminal, &g_shell.shell_tmodes);
  }

  /* Setup signal handlers */
  setup_signals();

  /* Initialize jobs table */
  init_jobs();
}

void cleanup_shell(void) {
  int i;

  /* Wait for all jobs to finish or kill them */
  for (i = 0; i < g_shell.job_count; i++) {
    if (g_shell.jobs[i].state == JOB_RUNNING ||
        g_shell.jobs[i].state == JOB_STOPPED) {
      kill(-g_shell.jobs[i].pgid, SIGTERM);
    }
  }

  /* Restore terminal */
  if (g_shell.is_interactive) {
    tcsetattr(g_shell.shell_terminal, TCSADRAIN, &g_shell.shell_tmodes);
  }
}

void print_prompt(void) {
  if (g_shell.is_interactive) {
    printf("seal> ");
    fflush(stdout);
  }
}
