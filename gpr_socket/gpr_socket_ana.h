#ifndef gpr_socket_ana__h
#define gpr_socket_ana__h

#include <stdbool.h>

struct ThreadSendFileInfo
{
	char socketCode;
    char *path;
    FILE *filePtr;
};

void sendRootDir();
void sendDiskSize();
void sendReadDir(char *path, char protocol, bool repeat);
void sendDeleteFile(char *path);
void sendDeleteFolder(char *path);
void sendUsbInFo();
void *tryCopyFiles(void *arg);
void *sendFileData(void *arg);
void sendLoadConfiFile(char *path);
void sendSaveConfigFile(char *bytes);

void fileSendCleanUp(void *arg);
void threadSendFileData(char *path, char responeSocketCode);
void threadSendFileCancel();

void usbCopyCleanUp(void *arg);
void threadUsbCopy(char *bytes);
void threadUsbCopyCancel();
#endif