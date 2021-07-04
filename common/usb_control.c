#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "usb_control.h"
#include "dir_control.h"
#include "gpr_param.h"

struct UsbData usbData;
const int deviceNameCnt = 10;
char *deviceName[10] = {"sda", "sdb", "sdc", "sdd", "sde", "sdf", "sdg", "sdh", "sdi", "sdj"};

bool initUsbMountPath()
{
    char *usbPath = "/usb";
    strcpy(usbData.mountPath, strRealPath);
    strcat(usbData.mountPath, usbPath);
    mkdirs(usbData.mountPath);
    return true;
}

char *getUsbInfo()
{
    tryUsbUmount();
    char *out = NULL;
    for (int i = 0; i < deviceNameCnt; i++)
    {
        if (findUsb(deviceName[i]) && tryUsbMount())
        {
            printf("find USB: %s\n", usbData.diskModel);
            char path[15];
            strcpy(path, "/dev/");
            strcat(path, deviceName[i]);
            strcat(path, "1");
            cJSON *root = getDiskSize(path);
            if (root != NULL)
            {
                printf("usb disk size\n");
                cJSON_AddItemToObject(root, "model", cJSON_CreateString(usbData.diskModel));
                out = cJSON_Print(root);
                cJSON_Delete(root);
            }
            tryUsbUmount();
            break;
        }
    }

    return out;
}

bool findUsb(char name[])
{
    bool isFind = false;
    char command[100] = "sudo lsblk -o MODEL,NAME,FSTYPE | grep ";
    strcat(command, name);
    FILE *file = popen(command, "r");
    if (file != NULL)
    {
        char buf[100];
        //ex) Storage_Device sda
        if (fgets(buf, 100, file) != NULL)
        {
            strcpy(usbData.deviceName, name);
            char *ptr = strtok(buf, " ");
            if (ptr != NULL)
            {
                strcpy(usbData.diskModel, ptr);
            }
            //ex)└─sdb1      vfat
            if (fgets(buf, 100, file) != NULL)
            {
                if (strstr(buf, "ext3") != NULL)
                {
                    strcpy(usbData.fsType, "ext3");
                    isFind = true;
                }
                else if (strstr(buf, "ext4") != NULL)
                {
                    strcpy(usbData.fsType, "ext4");
                    isFind = true;
                }
                else if (strstr(buf, "ntfs") != NULL)
                {
                    strcpy(usbData.fsType, "ntfs");
                    isFind = true;
                }
                else if (strstr(buf, "vfat") != NULL)
                {
                    strcpy(usbData.fsType, "vfat");
                    isFind = true;
                }
                else if (strstr(buf, "exfat") != NULL)
                {
                    strcpy(usbData.fsType, "exfat");
                    isFind = true;
                }
            }
        }
        pclose(file);
    }
    return isFind;
}

bool tryUsbMount()
{
    if (usbData.fsType[0] == 0)
    {
        return false;
    }
    mkdirs(usbData.mountPath);

    char usbPath[15];
    strcpy(usbPath, "/dev/");
    strcat(usbPath, usbData.deviceName);
    strcat(usbPath, "1");
    int mounting = mount(usbPath, usbData.mountPath, usbData.fsType, MS_SYNCHRONOUS | MS_DIRSYNC, NULL);

    if (mounting == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void tryUsbUmount()
{
    int umounting = umount(usbData.mountPath);
    char command[50];
    strcpy(command, "umount -l ");
    strcat(command, usbData.mountPath);
    FILE *file = popen(command, "r");
    pclose(file);
}

char *changeSDPathToUsbPath(char *sdPath)
{
    char *restPath = sdPath + strlen(strRealPath);
    if (restPath != NULL)
    {
        int path_len = strlen(usbData.mountPath) + strlen(restPath) + 1;
        char *movePath = (char *)malloc(path_len);
        strcpy(movePath, usbData.mountPath);
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
    bool result = copyFile(usbPath, path);
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