#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>

#include "gpr_socket_ana.h"
#include "gpr_socket.h"
#include "gpr_socket_protocol.h"
#include "../common/cJSON.h"
#include "../common/dir_control.h"
#include "../common/gpr_param.h"
#include "../common/usb_control.h"

void sendRootDir()
{
    int pathLen = strlen(strRealPath) + strlen(fixDataRootName) + 1;
    char pathBuf[pathLen];
    strcpy(pathBuf, strRealPath);
    strcat(pathBuf, "/");
    strcat(pathBuf, fixDataRootName);
    socket_write(ANA_ROOT_DIR_NTF, pathBuf, strlen(pathBuf));
}

void sendDiskSize()
{
    char *out = getDiskSize("/dev/root", false);
    socket_write(ANA_DISK_SIZE_NTF, out, strlen(out));
    free(out);
}

void sendReadDir(char *path, char protocol, bool repeat)
{
    cJSON *root;
    root = cJSON_CreateArray();
    addDirInfo(root, path);
    getDirList(root, path, repeat);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);

    socket_write(protocol, out, strlen(out));
    free(out);
}

void sendDeleteFile(char *path)
{
    deleteFile(path);

    cJSON *list = cJSON_CreateArray();
    cJSON_AddItemToArray(list, cJSON_CreateString(path));
    char *out = cJSON_Print(list);
    cJSON_Delete(list);

    socket_write(ANA_DELETE_FILE_NTF, out, strlen(out));
    free(out);
}

void sendDeleteFolder(char *path)
{
    cJSON *list = cJSON_CreateArray();
    getDirList(list, path, true);

    //삭제
    deleteDirList(list);
    deleteDir(path);

    //삭제한 파일의 경로만 보내줌
    cJSON *send_list = cJSON_CreateArray();
    cJSON_AddItemToArray(send_list, cJSON_CreateString(path));
    for (int i = cJSON_GetArraySize(list) - 1; i >= 0; i--)
    {
        cJSON *subitem = cJSON_GetArrayItem(list, i);
        char *tempPath = cJSON_GetObjectItem(subitem, "path")->valuestring;
        cJSON_AddItemToArray(send_list, cJSON_CreateString(tempPath));
    }
    cJSON_Delete(list);

    char *out = cJSON_Print(send_list);
    cJSON_Delete(send_list);
    socket_write(ANA_DELETE_FOLDER_NTF, out, strlen(out));

    // printf("%s \n", out);
    free(out);
}

void sendUsbInFo()
{
    if (tryUsbMount())
    {
        char *out = getDiskSize("/dev/sda", true);
        socket_write(ANA_USB_INFO_NTF, out, strlen(out));
        free(out);
    }
    else
    {
        socket_write(ANA_USB_INFO_FAILED_NTF, "", 0);
    }
    tryUsbUmount();
}

void tryCopyFiles(char *bytes)
{
    if (tryUsbMount())
    {
        cJSON *list = cJSON_Parse(bytes);
        for (int i = cJSON_GetArraySize(list) - 1; i >= 0; i--)
        {
            cJSON *subitem = cJSON_GetArrayItem(list, i);
            char *path = cJSON_GetObjectItem(subitem, "path")->valuestring;
            char *subPath = path + strlen(strRealPath);
            socket_write(ANA_USB_COPYING_NAME_NTF, subPath, strlen(subPath));
            if (cJSON_GetObjectItem(subitem, "isDir")->valueint)
            {
                copyFolderToUsb(path);
            }
            else
            {
                char *name = cJSON_GetObjectItem(subitem, "name")->valuestring;
                if (copyFileToUsb(path, name) == false)
                {
                    socket_write(ANA_USB_COPY_FAILED_NAME_NTF, subPath, strlen(subPath));
                }
            }
        }
        cJSON_Delete(list);
        tryUsbUmount();
    }
    else
    {
        socket_write(ANA_USB_INFO_FAILED_NTF, "", 0);
    }
}