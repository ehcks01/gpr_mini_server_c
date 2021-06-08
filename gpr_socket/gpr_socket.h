#ifndef gpr_socket__h
#define gpr_socket__h

#include <stdbool.h>

#define PORT 4000
extern int server_socket, client_socket;

bool socket_ready();
int socket_client_accept();
int socket_receive(char *buff_rcv);
void socket_client_done();
void socket_server_done();
void socket_write(char code, void *bytes, int size);
void sendSavePath();
void socket_read(unsigned char buffer[], int buff_size);
void socket_close();

#endif