#ifndef SHELL_H
#define SHELL_H

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 64
#define MAX_TOKENS 128

/* Job states */
typedef enum { JOB_RUNNING, JOB_STOPPED, JOB_DONE } JobState;

/* Redirection types */
typedef enum {
  REDIR_NONE,
  REDIR_IN,     /* < */
  REDIR_OUT,    /* > */
  REDIR_APPEND, /* >> */
  REDIR_ERR,    /* 2> */
  REDIR_ERR_OUT /* 2>&1 */
} RedirType;

/* Redirection structure */
typedef struct {
  RedirType type;
  char *filename;
  int fd;
} Redirection;

/* Command structure */
typedef struct {
  char **argv;         /* Command arguments */
  int argc;            /* Argument count */
  Redirection *redirs; /* Array of redirections */
  int redir_count;     /* Number of redirections */
  int background;      /* Background flag */
} Command;

/* Pipeline structure */
typedef struct {
  Command *commands; /* Array of commands */
  int cmd_count;     /* Number of commands in pipeline */
} Pipeline;

/* Job structure */
typedef struct {
  int job_id;       /* Job ID */
  pid_t pgid;       /* Process group ID */
  char *command;    /* Command string */
  JobState state;   /* Job state */
  int saved_stdin;  /* Saved stdin for fg/bg */
  int saved_stdout; /* Saved stdout for fg/bg */
  int saved_stderr; /* Saved stderr for fg/bg */
} Job;

/* Global shell state */
typedef struct {
  Job jobs[MAX_JOBS];          /* Jobs table */
  int job_count;               /* Number of active jobs */
  pid_t shell_pgid;            /* Shell process group ID */
  int shell_terminal;          /* Shell's controlling terminal */
  int is_interactive;          /* Interactive mode flag */
  struct termios shell_tmodes; /* Shell terminal modes */
} ShellState;

/* Global shell state instance */
extern ShellState g_shell;

/* Lexer functions */
char **tokenize(const char *line, int *token_count);
void free_tokens(char **tokens, int count);

/* Parser functions */
Pipeline *parse_pipeline(char **tokens, int token_count);
void free_pipeline(Pipeline *pipeline);

/* Executor functions */
int execute_pipeline(Pipeline *pipeline);
int execute_command(Command *cmd, int is_pipe, int in_fd, int out_fd);

/* Redirection functions */
int setup_redirections(Redirection *redirs, int count, int *saved_fds);
void restore_redirections(int *saved_fds, int count);

/* Job control functions */
void init_jobs(void);
int add_job(pid_t pgid, const char *command, JobState state);
void remove_job(int job_id);
Job *get_job(int job_id);
Job *find_job_by_pgid(pid_t pgid);
void update_job_state(pid_t pgid, JobState state);
void list_jobs(void);
int bring_job_to_foreground(int job_id, int cont);
int send_job_to_background(int job_id, int cont);

/* Signal handling functions */
void setup_signals(void);
void block_signals(void);
void unblock_signals(void);

/* Built-in commands */
int is_builtin(const char *cmd);
int execute_builtin(Command *cmd);
int builtin_cd(char **argv);
int builtin_exit(char **argv);
int builtin_jobs(char **argv);
int builtin_fg(char **argv);
int builtin_bg(char **argv);
int builtin_help(char **argv);
int builtin_export(char **argv);

/* Utility functions */
char *trim(char *str);
void print_error(const char *msg);
void print_prompt(void);

/* Shell initialization */
void init_shell(void);
void cleanup_shell(void);

#endif /* SHELL_H */
