#ifndef CMDSTRUCT_H
#define CMDSTRUCT_H

#define ARGS_MAX			512

#ifndef DEBUG
#define DEBUG					0
#endif

typedef struct fgexit
{
  int status;
  int signal;
} Fgexit;


typedef struct cmd
{
	//is this a comment?
	int comment;
  //is this a builtin?
  int builtin;
  //are we to run this in the background?
  int bg;
  //if > -1, the index of the src of redirection for stdin
  int redir_in;
  //if > -1, the index of the destination for stdout
  int redir_out;
  //if > -1, the pid of the smallsh
  int pidarg;
  //where in the arg list does the PID go? 
  int pidarg_idx;
  //arg count
  int cmd_argc;
  //the command 
  char *the_cmd;
  //the_cmd args
  char *cmd_args[ARGS_MAX];

} Cmd;

void init_cmd_struct(struct cmd *cs);

void print_cmd_struct(struct cmd *cs);

#endif 
