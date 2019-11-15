#include <string.h>
#include <stdlib.h>

#ifndef CMDSTRUCT_H
#include "cmdstruct.h"
#define CMDSTRUCT_H
#endif

void init_cmd(struct cmd *cs)
{
	cs->comment = -1;
  cs->builtin = -1;
  cs->bg = -1;
  cs->redir_in = -1;
  cs->redir_out = -1;
  cs->pidarg = -1;
  cs->pidarg_idx = -1;
  cs->cmd_argc = -1;
  cs->the_cmd = NULL;
  memset(cs->cmd_args, '\0', ARGS_MAX);
}
