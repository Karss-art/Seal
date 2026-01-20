#include "shell.h"

int is_builtin(const char *cmd) {
  return (strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0 ||
          strcmp(cmd, "jobs") == 0 || strcmp(cmd, "fg") == 0 ||
          strcmp(cmd, "bg") == 0 || strcmp(cmd, "help") == 0 ||
          strcmp(cmd, "export") == 0);
}

int execute_builtin(Command *cmd) {
  if (strcmp(cmd->argv[0], "cd") == 0) {
    return builtin_cd(cmd->argv);
  } else if (strcmp(cmd->argv[0], "exit") == 0) {
    return builtin_exit(cmd->argv);
  } else if (strcmp(cmd->argv[0], "jobs") == 0) {
    return builtin_jobs(cmd->argv);
  } else if (strcmp(cmd->argv[0], "fg") == 0) {
    return builtin_fg(cmd->argv);
  } else if (strcmp(cmd->argv[0], "bg") == 0) {
    return builtin_bg(cmd->argv);
  } else if (strcmp(cmd->argv[0], "help") == 0) {
    return builtin_help(cmd->argv);
  } else if (strcmp(cmd->argv[0], "export") == 0) {
    return builtin_export(cmd->argv);
  }

  return -1;
}

int builtin_cd(char **argv) {
  const char *dir;

  if (argv[1] == NULL) {
    /* No argument, go to HOME */
    dir = getenv("HOME");
    if (dir == NULL) {
      print_error("cd: HOME not set");
      return -1;
    }
  } else {
    dir = argv[1];
  }

  if (chdir(dir) < 0) {
    perror("cd");
    return -1;
  }

  return 0;
}

int builtin_exit(char **argv) {
  int status = 0;

  if (argv[1] != NULL) {
    status = atoi(argv[1]);
  }

  cleanup_shell();
  exit(status);
}

int builtin_jobs(char **argv) {
  list_jobs();
  return 0;
}

int builtin_fg(char **argv) {
  int job_id;

  if (argv[1] == NULL) {
    /* Get most recent job */
    job_id = g_shell.job_count;
    if (job_id == 0) {
      print_error("fg: no current job");
      return -1;
    }
  } else {
    job_id = atoi(argv[1]);
  }

  return bring_job_to_foreground(job_id, 1);
}

int builtin_bg(char **argv) {
  int job_id;

  if (argv[1] == NULL) {
    /* Get most recent stopped job */
    int i;
    job_id = -1;
    for (i = MAX_JOBS - 1; i >= 0; i--) {
      if (g_shell.jobs[i].job_id != 0 && g_shell.jobs[i].state == JOB_STOPPED) {
        job_id = g_shell.jobs[i].job_id;
        break;
      }
    }

    if (job_id == -1) {
      print_error("bg: no stopped jobs");
      return -1;
    }
  } else {
    job_id = atoi(argv[1]);
  }

  return send_job_to_background(job_id, 1);
}

int builtin_help(char **argv) {
  printf("Seal Shell - Custom Shell with Job Control\n\n");
  printf("Built-in commands:\n");
  printf("  cd [dir]       Change directory\n");
  printf("  exit [status]  Exit shell\n");
  printf("  jobs           List active jobs\n");
  printf("  fg [job_id]    Bring job to foreground\n");
  printf("  bg [job_id]    Send job to background\n");
  printf("  help           Show this help\n");
  printf("  export VAR=val Set environment variable\n\n");
  printf("Redirection operators:\n");
  printf("  <              Redirect input\n");
  printf("  >              Redirect output (truncate)\n");
  printf("  >>             Redirect output (append)\n");
  printf("  2>             Redirect stderr\n");
  printf("  2>&1           Redirect stderr to stdout\n");
  printf("  |              Pipe\n\n");
  printf("Job control:\n");
  printf("  &              Run command in background\n");
  printf("  Ctrl-C         Send SIGINT to foreground job\n");
  printf("  Ctrl-Z         Send SIGTSTP to foreground job\n");

  return 0;
}

int builtin_export(char **argv) {
  if (argv[1] == NULL) {
    print_error("export: missing argument");
    return -1;
  }

  /* Parse VAR=value */
  char *eq = strchr(argv[1], '=');
  if (eq == NULL) {
    print_error("export: invalid syntax (use VAR=value)");
    return -1;
  }

  *eq = '\0';
  char *name = argv[1];
  char *value = eq + 1;

  if (setenv(name, value, 1) < 0) {
    perror("setenv");
    return -1;
  }

  return 0;
}
