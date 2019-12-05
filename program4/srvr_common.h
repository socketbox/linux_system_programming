#ifndef SRVR_COMMON_H
#define SRVR_COMMON_H

typedef struct child_processes
{
	int count;
	pid_t kids[5];
} Children;

int check_client(int sock, int client_type);
char* get_clients_file(int sock, int filesz);
int get_file_len(int cxfd);

#endif
