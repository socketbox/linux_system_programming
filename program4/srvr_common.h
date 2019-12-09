#ifndef SRVR_COMMON_H
#define SRVR_COMMON_H

#include <signal.h>

//global for tracking child count
extern sig_atomic_t KIDCOUNT;

int check_client(int sock, int client_type);
char* get_clients_file(int sock, int filesz);
int get_file_len(int cxfd);
void set_srvr_proc_mask();
void chld_hndlr(int sig, siginfo_t *sigi, void *ucontext);
void reg_chld_handler();

#endif
