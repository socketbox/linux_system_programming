#ifndef SMSHCHILD_H
#define SMSHCHILD_H
#include "cmdstruct.h"

pid_t run_fg_child(Cmd *cs, Fgexit *fg);
int redir_stdout(Cmd *cs);
int redir_stdin(Cmd *cs);
void prep_args(Cmd *cs, char *arg_arr[]);
void exec_fg_cmd(Cmd *cs, char *arg_arr[]);

#endif
