#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "usb_control.h"
#include "dir_control.h"
#include "gpr_param.h"

//라즈베리에 usb가 제대로 언마운트가 안되면 다음 마운트 경로는 sda->sdb로 순차적으로 넘어감. 그래서 최대 10대 까지만 조회하게함.
struct UsbData usbData;
const int deviceNameCnt = 10;
char *deviceName[10] = {"sda", "sdb", "sdc", "sdd", "sde", "sdf", "sdg", "sdh", "sdi", "sdj"};

//usb가 마운트되면 usb 데이터와 연결되는 라즈베리 OS 경로
bool initUsbMountPath()
{
    char *usbPath = "/usb";
    strcpy(usbData.mountPath, strRealPath); //프로그램 실행경로
    strcat(usbData.mountPath, usbPath);
    mkdirs(usbData.mountPath);
    return true;
}

//연결된 usb를 찾으면 정보를 확인. 처음 발견한 usb 하나만 조회
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

//usb 디바이스 이름을 라즈베리에서 조회
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


//usb가 조회되면, '프로그램 실행경로/usb' 폴더로 마운트 시도
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

//마운트된 usb를 제거할 수 있게 언마운트
void tryUsbUmount()
{
    int umounting = umount(usbData.mountPath);
    char command[50];
    strcpy(command, "umount -l ");
    strcat(command, usbData.mountPath);
    FILE *file = popen(command, "r");
    pclose(file);
}

//데이터 경로를 마운트한 usb 경로로 변환
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

//파일을 마운트한 usb로 복사
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

//폴더를 마운트한 usb로 복사
void copyFolderToUsb(char *path)
{
    //copy라고 쓰고.. 폴더를 만듬
    char *usbPath = changeSDPathToUsbPath(path);
    mkdirs(usbPath);
    free(usbPath);
}