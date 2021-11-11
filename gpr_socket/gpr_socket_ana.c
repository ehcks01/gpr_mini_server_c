#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>

#include "gpr_socket_ana.h"
#include "gpr_socket.h"
#include "gpr_socket_protocol.h"
#include "gpr_socket_data.h"
#include "../common/cJSON.h"
#include "../common/dir_control.h"
#include "../common/gpr_param.h"
#include "../common/usb_control.h"

pthread_t fileThread;
pthread_t usbThread;

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
    cJSON *root = getDiskSize("/root");
    char *out = cJSON_Print(root);
    socket_write(ANA_DISK_SIZE_NTF, out, strlen(out));
    cJSON_Delete(root);
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
    char *out = getUsbInfo();
    if (out != NULL)
    {
        socket_write(ANA_USB_INFO_NTF, out, strlen(out));
        free(out);
    }
    else
    {
        socket_write(ANA_USB_INFO_FAILED_NTF, "", 0);
    }
}

void *tryCopyFiles(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push(usbCopyCleanUp, arg);

    if (tryUsbMount())
    {
        cJSON *list = arg;
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
        socket_write(ANA_USB_COPY_DONE_NTF, "", 0);
    }
    else
    {
        socket_write(ANA_USB_INFO_FAILED_NTF, "", 0);
    }
    pthread_cleanup_pop(arg);
}

void fileSendCleanUp(void *arg)
{
    struct ThreadSendFileInfo *sendInfo = arg;
    free(sendInfo->path);
    fclose(sendInfo->filePtr);
    free(sendInfo);
}

void threadSendFileCancel()
{
    pthread_cancel(fileThread);
}

void *sendFileData(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push(fileSendCleanUp, arg);

    int filelen, fileLentDivideCnt, fileLentDivideRest, sendBufferSize = 2048;
    char fileSizeToBytes[4];
    char sendBuffer[sendBufferSize];

    struct ThreadSendFileInfo *sendInfo = arg;
    printf("load filed: %s \n", sendInfo->path);
    sendInfo->filePtr = fopen(sendInfo->path, "rb");
    if (sendInfo->filePtr != NULL)
    {
        fseek(sendInfo->filePtr, 0, SEEK_END);
        filelen = ftell(sendInfo->filePtr);
        rewind(sendInfo->filePtr);

        //파일 사이즈를 먼저 보냄
        intToBytes(filelen, fileSizeToBytes, 4);
        socket_write(ANA_FILE_SIZE_NTF, fileSizeToBytes, 4);

        //파일 데이터를 쪼개서 보냄
        fileLentDivideCnt = filelen / sendBufferSize;
        fileLentDivideRest = filelen % sendBufferSize;
        for (int i = 0; i < fileLentDivideCnt; i++)
        {
            fread(sendBuffer, sendBufferSize, 1, sendInfo->filePtr);
            socket_write(ANA_FILE_DATA_NTF, sendBuffer, sendBufferSize);
        }
        if (fileLentDivideRest > 0)
        {
            fread(sendBuffer, fileLentDivideRest, 1, sendInfo->filePtr);
            socket_write(ANA_FILE_DATA_NTF, sendBuffer, fileLentDivideRest);
        }
    }
    socket_write(sendInfo->socketCode, "", 0);
    pthread_cleanup_pop(arg);
}

void sendLoadConfiFile(char *path)
{
    FILE *fileptr;
    char *buffer;
    int filelen;

    printf("load config: %s \n", path);
    fileptr = fopen(path, "r");
    if (fileptr != NULL)
    {
        fseek(fileptr, 0, SEEK_END);
        filelen = ftell(fileptr);
        rewind(fileptr);

        buffer = (char *)malloc(filelen * sizeof(char));
        fread(buffer, filelen, 1, fileptr);
        socket_write(ANA_LOAD_CONFIG_FILE_NTF, buffer, filelen);
        free(buffer);
        fclose(fileptr);
    }
    else
    {
        socket_write(ANA_LOAD_CONFIG_FILE_NTF, "", 0);
    }
}

void sendSaveConfigFile(char *bytes)
{
    cJSON *json = cJSON_Parse(bytes);
    if (json != NULL)
    {
        char *path = cJSON_GetObjectItem(json, "path")->valuestring;
        printf("save config: %s \n", path);
        char *content = cJSON_GetObjectItem(json, "content")->valuestring;

        FILE *fptr = fopen(path, "w+");
        if (fptr != NULL)
        {
            fwrite(content, strlen(content), 1, fptr);
            fclose(fptr);
        }
        cJSON_Delete(json);
    }
    socket_write(ANA_SAVE_CONFIG_FILE_NTF, "", 0);
}

void threadSendFileData(char *path, char responeSocketCode)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    struct ThreadSendFileInfo *sendInfo = malloc(sizeof(struct ThreadSendFileInfo));
    int pathLen = strlen(path);
    sendInfo->path = calloc(pathLen + 1, 1);
    strcpy(sendInfo->path, path);
    sendInfo->socketCode = responeSocketCode;

    pthread_create(&fileThread, &attr, sendFileData, sendInfo);
    pthread_attr_destroy(&attr);
}

void threadUsbCopy(char *bytes)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    cJSON *list = cJSON_Parse(bytes);
    pthread_create(&usbThread, &attr, tryCopyFiles, list);
    pthread_attr_destroy(&attr);
}

void usbCopyCleanUp(void *arg)
{
    cJSON_Delete(arg);
    tryUsbUmount();
}

void threadUsbCopyCancel()
{
    pthread_cancel(usbThread);
}