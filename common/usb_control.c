#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "usb_control.h"
#include "dir_control.h"
#include "gpr_param.h"

struct UsbData usbData;

bool initUsbMountPath()
{
    char *usbPath = "/usb";
    int pathlen = strlen(strRealPath) + strlen(usbPath) + 1;

    usbData.usbMountPath = (char *)calloc(pathlen, 1);
    strcpy(usbData.usbMountPath, strRealPath);
    strcat(usbData.usbMountPath, usbPath);

    mkdirs(usbData.usbMountPath);
    return true;
}

void setUsbFsType()
{
    FILE *file;
    char *buf;

    memset(usbData.usbFsType, 0, sizeof(usbData.usbFsType));
    file = popen("lsblk -f /dev/sda1", "r");
    if (file == NULL)
    {
        return;
    }
    buf = malloc(1024);
    
    while (fgets(buf, 1024, file) != NULL)
    {
        char *ptr = strstr(buf, "ext4");
        if (ptr != NULL)
        {
            memcpy(usbData.usbFsType, "ext4", strlen("ext4"));
            break;
        }
        ptr = strstr(buf, "ext3");
        if (ptr != NULL)
        {
            memcpy(usbData.usbFsType, "ext3", strlen("ext3"));
            break;
        }

        ptr = strstr(buf, "ntfs");
        if (ptr != NULL)
        {
            memcpy(usbData.usbFsType, "ntfs", strlen("ntfs"));
            break;
        }

        ptr = strstr(buf, "vfat");
        if (ptr != NULL)
        {
            memcpy(usbData.usbFsType, "vfat", strlen("vfat"));
            break;
        }

        ptr = strstr(buf, "exfat");
        if (ptr != NULL)
        {
            memcpy(usbData.usbFsType, "exfat", strlen("exfat"));
            break;
        }
    }

    // printf("usbData.usbFsType: %s \n", usbData.usbFsType);
    pclose(file);
    free(buf);
}

bool tryUsbMount()
{
    setUsbFsType();
    if (usbData.usbFsType[0] == 0)
    {
        return false;
    }
    mkdirs(usbData.usbMountPath);

    int mounting;
    mounting = mount("/dev/sda1", usbData.usbMountPath, usbData.usbFsType, MS_SYNCHRONOUS | MS_DIRSYNC, NULL);

    if (mounting == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool tryUsbUmount()
{
    int umounting = umount(usbData.usbMountPath);
    if (umounting == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

char *getUsbDiskModel()
{
    FILE *f;
    char *buf, *out;

    f = popen("fdisk -l /dev/sda", "r");
    if (f == NULL)
    {
        return NULL;
    }
    buf = malloc(1024);

    while (fgets(buf, 1024, f) != NULL)
    {
        char *ptr = strstr(buf, "Disk model:  ");
        if (ptr != NULL)
        {
            char *temp = ptr + strlen("Disk model:  ");
            out = malloc(strlen(temp));
            strcpy(out, temp);
            break;
        }
    }
    pclose(f);
    free(buf);

    return out;
}

char *changeSDPathToUsbPath(char *sdPath)
{
    char *restPath = sdPath + strlen(strRealPath);
    if (restPath != NULL)
    {
        int path_len = strlen(usbData.usbMountPath) + strlen(restPath) + 1;
        char *movePath = (char *)malloc(path_len);
        strcpy(movePath, usbData.usbMountPath);
        strcat(movePath, restPath);
        return movePath;
    }

    return NULL;
}

bool copyFileToUsb(char *path, char *name)
{
    //1. 저장된 경로에서 파일이름을 제외한 폴더 경로만 추출
    int dirLen = strlen(path) - strlen(name) - 1;
    char dir[dirLen + 1];
    dir[dirLen] = 0;
    strncpy(dir, path, dirLen);

    //2. 폴더를 usb에 생성
    char *usbPath = changeSDPathToUsbPath(dir);
    mkdirs(usbPath);
    free(usbPath);

    //3. 파일을 usb로 복사
    usbPath = changeSDPathToUsbPath(path);
    bool result =  copyFile(usbPath, path);
    free(usbPath);

    return result;
}

void copyFolderToUsb(char *path)
{
    //copy라고 쓰고.. 폴더를 만듬
    char *usbPath = changeSDPathToUsbPath(path);
    mkdirs(usbPath);
    free(usbPath);
}