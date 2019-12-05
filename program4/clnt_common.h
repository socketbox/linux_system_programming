#ifndef CLNT_COMMON_H
#define CLNT_COMMON_H

int verify_file(char *fn);
char* recv_bytes(int socket, int buffsz);
int send_file(int socket, char *fname, int bytesz);
int send_file_size(int socket, int bytesz);

#endif
