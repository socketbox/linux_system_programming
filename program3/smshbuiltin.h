#ifndef SMSHBUILTIN_H
#define SMSHBUILTIN_H
#include "cmdstruct.h"

void run_exit(int pid_arr[]);
void run_cd(Cmd *cs);
void run_status(Fgexit *fge);

#endif
