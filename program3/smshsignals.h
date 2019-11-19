#ifndef SMSHSIGNALS_H
#define SMSHSIGNALS_H

#include <signal.h>

extern sig_atomic_t STOPPED;

void set_bg_mask();
void set_fg_mask();
void set_smsh_mask();
void handle_int(int sig);
void handle_tstp(int sig);
void handle_chld(int sig);
void reg_fg_handlers();
void reg_smsh_handlers();

#endif
