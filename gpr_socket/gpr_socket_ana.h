#ifndef gpr_socket_ana__h
#define gpr_socket_ana__h

#include <stdbool.h>

void sendRootDir();
void sendDiskSize();
void sendReadDir(char *path, char protocol, bool repeat);
void sendDeleteFile(char *path);
void sendDeleteFolder(char *path);
void sendUsbInFo();
void tryCopyFiles(char *bytes);

#endif