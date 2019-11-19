#ifndef SMSHCHILD_H
#define SMSHCHILD_H

#include "cmdstruct.h"

void drain(int fd);
void free_exec_args(int arr_len, char *arr[]);
void run_bg_child(Cmd *cs);
pid_t run_fg_child(Cmd *cs, Fgexit *fg);
int redir_stdout(Cmd *cs);
int redir_stdin(Cmd *cs);
void prep_args(Cmd *cs, char *arg_arr[]);
void exec_cmd(Cmd *cs, char *arg_arr[]);

#endif
