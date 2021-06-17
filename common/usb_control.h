#ifndef usb_control__h
#define usb_control__h

#include <stdbool.h>

struct UsbData
{
    char *usbMountPath;
    char usbFsType[6];
};

extern struct UsbData usbData;

bool initUsbMountPath();
void setUsbFsType();
bool tryUsbMount();
bool tryUsbUmount();
char *getUsbDiskModel();
char *changeSDPathToUsbPath();
bool copyFileToUsb(char *path, char *name);
void copyFolderToUsb(char *path);

#endif