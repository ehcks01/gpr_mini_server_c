#ifndef usb_control__h
#define usb_control__h

#include <stdbool.h>

//마운트한 usb 정보
struct UsbData
{
    char mountPath[100]; //mount 되는 경로
    char deviceName[4]; //sda 같은 네임

    char fsType[6]; //포맷 타입
    char diskModel[100]; //usb model 이름
};

bool initUsbMountPath();
bool findUsb(char name[]);
char *getUsbInfo();
bool tryUsbMount();
void tryUsbUmount();
char *changeSDPathToUsbPath();
bool copyFileToUsb(char *path, char *name);
void copyFolderToUsb(char *path);

#endif