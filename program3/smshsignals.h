#ifndef SMSHSIGNALS_H
#define SMSHSIGNALS_H

void handle_int(int sig);
void handle_chld(int sig);
void reg_handlers();
#endif
