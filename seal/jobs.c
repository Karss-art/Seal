#include "shell.h"

void init_jobs(void) {
  int i;
  for (i = 0; i < MAX_JOBS; i++) {
    g_shell.jobs[i].job_id = 0;
    g_shell.jobs[i].pgid = 0;
    g_shell.jobs[i].command = NULL;
    g_shell.jobs[i].state = JOB_DONE;
  }
  g_shell.job_count = 0;
}

int add_job(pid_t pgid, const char *command, JobState state) {
  int i;

  /* Find free slot */
  for (i = 0; i < MAX_JOBS; i++) {
    if (g_shell.jobs[i].job_id == 0) {
      g_shell.jobs[i].job_id = i + 1;
      g_shell.jobs[i].pgid = pgid;
      g_shell.jobs[i].command = strdup(command);
      g_shell.jobs[i].state = state;
      g_shell.job_count++;
      return i + 1;
    }
  }

  print_error("jobs table full");
  return -1;
}

void remove_job(int job_id) {
  if (job_id < 1 || job_id > MAX_JOBS)
    return;

  int idx = job_id - 1;
  if (g_shell.jobs[idx].job_id == 0)
    return;

  if (g_shell.jobs[idx].command) {
    free(g_shell.jobs[idx].command);
  }

  g_shell.jobs[idx].job_id = 0;
  g_shell.jobs[idx].pgid = 0;
  g_shell.jobs[idx].command = NULL;
  g_shell.jobs[idx].state = JOB_DONE;
  g_shell.job_count--;
}

Job *get_job(int job_id) {
  if (job_id < 1 || job_id > MAX_JOBS)
    return NULL;

  int idx = job_id - 1;
  if (g_shell.jobs[idx].job_id == 0)
    return NULL;

  return &g_shell.jobs[idx];
}

Job *find_job_by_pgid(pid_t pgid) {
  int i;
  for (i = 0; i < MAX_JOBS; i++) {
    if (g_shell.jobs[i].job_id != 0 && g_shell.jobs[i].pgid == pgid) {
      return &g_shell.jobs[i];
    }
  }
  return NULL;
}

void update_job_state(pid_t pgid, JobState state) {
  Job *job = find_job_by_pgid(pgid);
  if (job) {
    job->state = state;
  }
}

void list_jobs(void) {
  int i;
  for (i = 0; i < MAX_JOBS; i++) {
    if (g_shell.jobs[i].job_id != 0) {
      const char *state_str;
      switch (g_shell.jobs[i].state) {
      case JOB_RUNNING:
        state_str = "Running";
        break;
      case JOB_STOPPED:
        state_str = "Stopped";
        break;
      case JOB_DONE:
        state_str = "Done";
        break;
      default:
        state_str = "Unknown";
      }

      printf("[%d]  %s\t\t%s\n", g_shell.jobs[i].job_id, state_str,
             g_shell.jobs[i].command);
    }
  }
}

int bring_job_to_foreground(int job_id, int cont) {
  Job *job = get_job(job_id);
  if (!job) {
    print_error("no such job");
    return -1;
  }

  /* Give terminal to job */
  if (g_shell.is_interactive) {
    tcsetpgrp(g_shell.shell_terminal, job->pgid);
  }

  /* Continue if stopped */
  if (cont) {
    if (kill(-job->pgid, SIGCONT) < 0) {
      perror("kill");
      return -1;
    }
  }

  job->state = JOB_RUNNING;

  /* Wait for job */
  int status;
  pid_t pid;

  while (1) {
    pid = waitpid(-job->pgid, &status, WUNTRACED);

    if (pid < 0) {
      if (errno == ECHILD) {
        /* Job finished */
        remove_job(job_id);
        break;
      }
      if (errno == EINTR) {
        continue;
      }
      perror("waitpid");
      break;
    }

    if (WIFSTOPPED(status)) {
      /* Job stopped */
      job->state = JOB_STOPPED;
      printf("\n[%d]+ Stopped %s\n", job_id, job->command);
      break;
    }

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      /* Check if all processes done */
      if (waitpid(-job->pgid, &status, WNOHANG) <= 0) {
        remove_job(job_id);
        break;
      }
    }
  }

  /* Give terminal back to shell */
  if (g_shell.is_interactive) {
    tcsetpgrp(g_shell.shell_terminal, g_shell.shell_pgid);
  }

  return 0;
}

int send_job_to_background(int job_id, int cont) {
  Job *job = get_job(job_id);
  if (!job) {
    print_error("no such job");
    return -1;
  }

  if (cont) {
    if (kill(-job->pgid, SIGCONT) < 0) {
      perror("kill");
      return -1;
    }
    printf("[%d]+ %s &\n", job_id, job->command);
  }

  job->state = JOB_RUNNING;

  return 0;
}
