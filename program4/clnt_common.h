#ifndef CLNT_COMMON_H
#define CLNT_COMMON_H

int verify_file(char *fn);
int recv_enc_bytes(int socket, char *buff, int buffsz);
int send_file(int socket, char *fname, int bytesz);
int send_file_size(int socket, int bytesz);

#endif
