#include "shell.h"

int setup_redirections(Redirection *redirs, int count, int *saved_fds) {
  if (!redirs || count == 0)
    return 0;

  for (int i = 0; i < count; i++) {
    Redirection *r = &redirs[i];
    int fd = -1;

    switch (r->type) {
    case REDIR_IN:
      /* Save stdin */
      saved_fds[0] = dup(STDIN_FILENO);
      /* Open file for reading */
      fd = open(r->filename, O_RDONLY);
      if (fd < 0) {
        perror(r->filename);
        return -1;
      }
      /* Redirect stdin */
      if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
      }
      close(fd);
      break;

    case REDIR_OUT:
      /* Save stdout */
      saved_fds[1] = dup(STDOUT_FILENO);
      /* Open file for writing (truncate) */
      fd = open(r->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0) {
        perror(r->filename);
        return -1;
      }
      /* Redirect stdout */
      if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
      }
      close(fd);
      break;

    case REDIR_APPEND:
      /* Save stdout */
      saved_fds[1] = dup(STDOUT_FILENO);
      /* Open file for writing (append) */
      fd = open(r->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (fd < 0) {
        perror(r->filename);
        return -1;
      }
      /* Redirect stdout */
      if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
      }
      close(fd);
      break;

    case REDIR_ERR:
      /* Save stderr */
      saved_fds[2] = dup(STDERR_FILENO);
      /* Open file for writing (truncate) */
      fd = open(r->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0) {
        perror(r->filename);
        return -1;
      }
      /* Redirect stderr */
      if (dup2(fd, STDERR_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
      }
      close(fd);
      break;

    case REDIR_ERR_OUT:
      /* Save stderr */
      saved_fds[2] = dup(STDERR_FILENO);
      /* Redirect stderr to stdout */
      if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
        perror("dup2");
        return -1;
      }
      break;

    default:
      break;
    }
  }

  return 0;
}

void restore_redirections(int *saved_fds, int count) {
  /* Restore stdin */
  if (saved_fds[0] >= 0) {
    dup2(saved_fds[0], STDIN_FILENO);
    close(saved_fds[0]);
    saved_fds[0] = -1;
  }

  /* Restore stdout */
  if (saved_fds[1] >= 0) {
    dup2(saved_fds[1], STDOUT_FILENO);
    close(saved_fds[1]);
    saved_fds[1] = -1;
  }

  /* Restore stderr */
  if (saved_fds[2] >= 0) {
    dup2(saved_fds[2], STDERR_FILENO);
    close(saved_fds[2]);
    saved_fds[2] = -1;
  }
}
