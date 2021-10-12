#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <wiringPi.h>
#include <sys/time.h>

#include "gpr_socket_acq.h"
#include "gpr_socket.h"
#include "gpr_socket_protocol.h"
#include "../common/gpr_param.h"
#include "../common/dir_control.h"
#include "../NVA/NVA_CON.h"
#include "../NVA/NVA6100.h"
#include "../encoder/encoder.h"

struct AcqContoroller acqCon = {.fp = NULL, .fileName = NULL, .savePath = NULL, .dataCnt = 0, .dataCnt = 0, .grid3D = NULL, .runAcq = false};

bool is2DScanMode()
{
    if (headerParameter.cScanMode == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isNotFull3DData()
{
    if (acqCon.dataCnt < acqCon.dataSize3D)
    {
        return true;
    }
    printf("data full: %d \n", acqCon.dataCnt);
    return false;
}

bool fileexists(const char *path, const char *filename)
{
    char *name = getFullPath(path, filename);
    bool retval = access(name, F_OK) == 0;
    free(name);
    return retval;
}

bool makeSavePath()
{
    char pathBuf[4096];
    strcpy(pathBuf, strRealPath);
    int inc = 0;
    strcat(pathBuf, "/");
    strcat(pathBuf, fixDataRootName);
    strcat(pathBuf, "/");
    strcat(pathBuf, headerParameter.strSiteName);

    if (is2DScanMode())
    {
        strcat(pathBuf, "/2D");
        free(acqCon.savePath);
        acqCon.savePath = (char *)calloc(strlen(pathBuf) + 1, 1);
        memcpy(acqCon.savePath, pathBuf, strlen(pathBuf));
        do
        {
            inc++;
            free(acqCon.fileName);
            //_001.MGM length 8
            acqCon.fileName = (char *)calloc(strlen(headerParameter.strOperator) + 8, 1);
            strcat(acqCon.fileName, headerParameter.strOperator);
            strcat(acqCon.fileName, "_");
            char snum[3];
            sprintf(snum, "%03d", inc);
            strcat(acqCon.fileName, snum);
            strcat(acqCon.fileName, ".MGM");
        } while (fileexists(acqCon.savePath, acqCon.fileName));

        printf("make savePath: %s \n", acqCon.savePath);
        printf("make fileName: %s \n", acqCon.fileName);
    }
    else
    {
        //001M length 4 (folder name: 001M1x1)
        char folderName[strlen(acqCon.grid3D) + 4];
        do
        {
            folderName[0] = 0;
            inc++;
            char snum[3];
            sprintf(snum, "%03d", inc);
            strcat(folderName, snum);
            strcat(folderName, "M");
            strcat(folderName, acqCon.grid3D);
        } while (fileexists(pathBuf, folderName));
        strcat(pathBuf, "/");
        strcat(pathBuf, folderName);
        free(acqCon.savePath);
        acqCon.savePath = (char *)calloc(strlen(pathBuf) + 1, 1);
        memcpy(acqCon.savePath, pathBuf, strlen(pathBuf));
        printf("make savePath: %s \n", acqCon.savePath);
    }

    return true;
}

void endFileWrite()
{
    if (acqCon.fp != NULL)
    {
        fclose(acqCon.fp);
        acqCon.fp = NULL;
    }
}

void deleteAcqFile()
{
    if (acqCon.savePath != NULL && acqCon.fileName != NULL)
    {
        char *path = getFullPath(acqCon.savePath, acqCon.fileName);
        deleteFile(path);
        free(path);
    }
}

void deleteAcq3DFolder()
{
    cJSON *list = cJSON_CreateArray();
    getDirList(list, acqCon.savePath, true);
    deleteDirList(list);
    deleteDir(acqCon.savePath); //자기 자신 삭제
    cJSON_Delete(list);
}

void stopAcq()
{
    endFileWrite();
    acqCon.runAcq = false;
}

void startAcq()
{
    struct stat st = {0};
    if (stat(acqCon.savePath, &st) == -1)
    {
        mkdirs(acqCon.savePath);
    }
    acqCon.dataCnt = 0;
    char *path = getFullPath(acqCon.savePath, acqCon.fileName);
    acqCon.fp = fopen(path, "wb");
    if (acqCon.fp != NULL)
    {
        fseek(acqCon.fp, fixHeaderSize, SEEK_SET);
    }
    free(path);
    acqCon.runAcq = true;
}

void frontRowData()
{
    //취득 시간
    struct timeval time;
    gettimeofday(&time, NULL);
    double start_ms = (double)time.tv_sec * 1000000 + (double)time.tv_usec;
    memcpy(acqCon.NVA_readData + fixDepthDataSize, &start_ms, sizeof(start_ms));

    if (acqCon.fp != NULL)
    {
        if (fwrite(acqCon.NVA_readData, 1, fixDepthDataSize, acqCon.fp) == fixDepthDataSize)
        {
            acqCon.dataCnt++;
            socket_write(ACQ_DATA_NTF, acqCon.NVA_readData, sizeof(acqCon.NVA_readData));
        }
    }
}

void backRowData()
{
    if (acqCon.fp != NULL)
    {
        long size = ftell(acqCon.fp);
        if (size > fixHeaderSize)
        {
            fseek(acqCon.fp, -fixDepthDataSize, SEEK_CUR);
            socket_write(ACQ_BACK_DATA_NTF, "", 0);

            acqCon.dataCnt--;
            if (acqCon.dataCnt < 0)
            {
                acqCon.dataCnt = 0;
            }
        }
    }
}

bool saveAcq(char *headerInfo, int size)
{
    bool saveState = false;
    char *path = getFullPath(acqCon.savePath, acqCon.fileName);
    FILE *fp = fopen(path, "r+");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_SET);
        //앞에 1바이트는 socket code라서 제외 시킴
        fwrite(headerInfo, 1, size - 1, fp);
        fclose(fp);
        saveState = true;
        printf("file save successed: %s \n", path);
    }
    else
    {
        printf("file save failed: %s \n", path);
    }
    free(path);
    return saveState;
}

void acqOn()
{
    digitalWrite(LASER_PIN, HIGH);
    digitalWrite(ENCODER_POWER_PIN, HIGH);
    NVA_Init(0);
    if (NVAParam.ChipID != 0x0306)
    {
        printf("Novelda initialization failed\n");
    }
}
void acqOff()
{
    digitalWrite(LASER_PIN, LOW);
    digitalWrite(ENCODER_POWER_PIN, LOW);
}
