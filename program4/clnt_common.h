#ifndef CLNT_COMMON_H
#define CLNT_COMMON_H

void send_client_type(int sock, int type);
void error(const char *msg);
void parse_response(int cxfd);
int verify_file(char *fn);
char* recv_bytes(int socket, int buffsz);
int send_file(int socket, char *fname, int bytesz);
int send_file_size(int socket, int bytesz);

#endif
