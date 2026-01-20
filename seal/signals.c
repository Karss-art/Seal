#include "shell.h"

static void sigchld_handler(int sig) {
  pid_t pid;
  int status;

  /* Reap all terminated/stopped children */
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
    Job *job = find_job_by_pgid(pid);

    if (job) {
      if (WIFEXITED(status) || WIFSIGNALED(status)) {
        /* Job terminated */
        job->state = JOB_DONE;
        if (g_shell.is_interactive) {
          printf("\n[%d]+ Done\t\t%s\n", job->job_id, job->command);
        }
        /* Will be removed on next cleanup */
      } else if (WIFSTOPPED(status)) {
        /* Job stopped */
        job->state = JOB_STOPPED;
      } else if (WIFCONTINUED(status)) {
        /* Job continued */
        job->state = JOB_RUNNING;
      }
    }
  }
}

void setup_signals(void) {
  struct sigaction sa;

  /* Setup SIGCHLD handler */
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  if (sigaction(SIGCHLD, &sa, NULL) < 0) {
    perror("sigaction");
    exit(1);
  }
}

void block_signals(void) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTSTP);
  sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblock_signals(void) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTSTP);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
