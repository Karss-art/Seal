#include "shell.h"

int execute_pipeline(Pipeline *pipeline) {
  if (!pipeline || pipeline->cmd_count == 0)
    return -1;

  /* Single command (no pipe) */
  if (pipeline->cmd_count == 1) {
    Command *cmd = &pipeline->commands[0];

    /* Check if built-in */
    if (is_builtin(cmd->argv[0])) {
      return execute_builtin(cmd);
    }

    /* Execute external command */
    return execute_command(cmd, 0, -1, -1);
  }

  /* Pipeline with multiple commands */
  int i;
  int pipefds[2];
  int prev_pipe = -1;
  pid_t pgid = 0;
  pid_t pid;
  int background = pipeline->commands[0].background;

  for (i = 0; i < pipeline->cmd_count; i++) {
    Command *cmd = &pipeline->commands[i];

    /* Create pipe for all but last command */
    int next_pipe = -1;
    if (i < pipeline->cmd_count - 1) {
      if (pipe(pipefds) < 0) {
        perror("pipe");
        return -1;
      }
      next_pipe = pipefds[0];
    }

    /* Fork child */
    pid = fork();
    if (pid < 0) {
      perror("fork");
      if (next_pipe >= 0) {
        close(pipefds[0]);
        close(pipefds[1]);
      }
      return -1;
    }

    if (pid == 0) {
      /* Child process */

      /* Set process group (first command creates group) */
      if (pgid == 0) {
        pgid = getpid();
      }
      setpgid(0, pgid);

      /* If not background and interactive, give terminal to process group */
      if (!background && g_shell.is_interactive) {
        tcsetpgrp(g_shell.shell_terminal, pgid);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
      }

      /* Setup redirections */
      int saved_fds[3] = {-1, -1, -1};
      if (setup_redirections(cmd->redirs, cmd->redir_count, saved_fds) < 0) {
        exit(1);
      }

      /* Setup pipe input */
      if (prev_pipe >= 0) {
        dup2(prev_pipe, STDIN_FILENO);
        close(prev_pipe);
      }

      /* Setup pipe output */
      if (next_pipe >= 0) {
        dup2(pipefds[1], STDOUT_FILENO);
        close(pipefds[0]);
        close(pipefds[1]);
      }

      /* Execute command */
      execvp(cmd->argv[0], cmd->argv);
      perror(cmd->argv[0]);
      exit(127);
    }

    /* Parent process */

    /* Set process group for first command */
    if (pgid == 0) {
      pgid = pid;
    }
    setpgid(pid, pgid);

    /* Close previous pipe */
    if (prev_pipe >= 0) {
      close(prev_pipe);
    }

    /* Close write end of current pipe */
    if (next_pipe >= 0) {
      close(pipefds[1]);
    }

    prev_pipe = next_pipe;
  }

  /* Add job if background */
  if (background) {
    /* Build command string */
    char cmd_str[MAX_LINE] = "";
    for (i = 0; i < pipeline->cmd_count; i++) {
      if (i > 0)
        strcat(cmd_str, " | ");
      strcat(cmd_str, pipeline->commands[i].argv[0]);
    }
    strcat(cmd_str, " &");

    add_job(pgid, cmd_str, JOB_RUNNING);
    printf("[%d] %d\n", g_shell.job_count, pgid);
  } else {
    /* Wait for foreground pipeline */
    int status;
    pid_t wait_pid;

    while (1) {
      wait_pid = waitpid(-pgid, &status, WUNTRACED);

      if (wait_pid < 0) {
        if (errno == ECHILD) {
          /* All children finished */
          break;
        }
        if (errno == EINTR) {
          /* Interrupted by signal, continue */
          continue;
        }
        perror("waitpid");
        break;
      }

      /* Check if stopped */
      if (WIFSTOPPED(status)) {
        /* Build command string */
        char cmd_str[MAX_LINE] = "";
        for (i = 0; i < pipeline->cmd_count; i++) {
          if (i > 0)
            strcat(cmd_str, " | ");
          strcat(cmd_str, pipeline->commands[i].argv[0]);
        }

        add_job(pgid, cmd_str, JOB_STOPPED);
        printf("\n[%d]+ Stopped %s\n", g_shell.job_count, cmd_str);
        break;
      }

      /* Check if all processes in group are done */
      if (WIFEXITED(status) || WIFSIGNALED(status)) {
        /* Check if there are more processes */
        if (waitpid(-pgid, &status, WNOHANG) == 0) {
          continue;
        }
      }
    }

    /* Give terminal back to shell */
    if (g_shell.is_interactive) {
      tcsetpgrp(g_shell.shell_terminal, g_shell.shell_pgid);
    }
  }

  return 0;
}

int execute_command(Command *cmd, int is_pipe, int in_fd, int out_fd) {
  pid_t pid;
  int status;

  /* Check if built-in */
  if (!is_pipe && is_builtin(cmd->argv[0])) {
    return execute_builtin(cmd);
  }

  /* Fork child */
  pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    /* Child process */

    /* Restore default signal handlers */
    if (g_shell.is_interactive) {
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
    }

    /* Setup redirections */
    int saved_fds[3] = {-1, -1, -1};
    if (setup_redirections(cmd->redirs, cmd->redir_count, saved_fds) < 0) {
      exit(1);
    }

    /* Execute command */
    execvp(cmd->argv[0], cmd->argv);
    perror(cmd->argv[0]);
    exit(127);
  }

  /* Parent process */
  if (!cmd->background) {
    /* Wait for foreground process */
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
    }
  } else {
    printf("[%d] %d\n", g_shell.job_count + 1, pid);
    add_job(pid, cmd->argv[0], JOB_RUNNING);
  }

  return 0;
}
