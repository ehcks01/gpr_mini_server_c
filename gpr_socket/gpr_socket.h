#ifndef gpr_socket__h
#define gpr_socket__h

#include <stdbool.h>

#define PORT 4000
extern int server_socket, client_socket, select_channel;
extern bool server_restart;

bool socket_ready();
int socket_client_accept();
int socket_receive(char *buff_rcv);
void socket_client_done();
void socket_server_done();
void socket_write(char code, char *bytes, int size);
void sendSavePath();
void socket_read(char buffer[], int buff_size);
void socket_close();
int tcpSetKeepAlive(int nSockFd_, int nKeepAlive_, int nKeepAliveIdle_, int nKeepAliveCnt_, int nKeepAliveInterval_);

#endif