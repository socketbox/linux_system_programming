#ifndef SMSHBUILTIN_H
#define SMSHBUILTIN_H
#include "cmdstruct.h"

void run_exit(Cmd cs);
void run_cd(Cmd *cs);
void run_status(Fgexit *fge, State *st);

#endif
