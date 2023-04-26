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

//취득 시 필요한 정보
struct AcqContoroller acqCon = {.fp = NULL, .fileName = NULL, .savePath = NULL, .dataCnt = 0, .dataCnt = 0, .grid3D = NULL, .runAcq = false};

//취득 모드가 2d인지 3d인지 확인
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

//3d 모드일 때 목표거리까지 취득했는지 확ㅇ니
bool isNotFull3DData()
{
    if (acqCon.dataCnt < acqCon.dataSize3D)
    {
        return true;
    }
    printf("data full: %d \n", acqCon.dataCnt);
    return false;
}

//파일이 존재하는지 확인
bool fileexists(const char *path, const char *filename)
{
    char *name = getFullPath(path, filename);
    bool retval = access(name, F_OK) == 0;
    free(name);
    return retval;
}

//취득파일 저장경로 생성. 파일 확장명은 mgm
bool makeSavePath()
{
    char pathBuf[4096];
    strcpy(pathBuf, strExePath);
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

//파일 쓰기 종료
void endFileWrite()
{
    if (acqCon.fp != NULL)
    {
        fclose(acqCon.fp);
        acqCon.fp = NULL;
    }
}

//취득 파일 삭제
void deleteAcqFile()
{
    if (acqCon.savePath != NULL && acqCon.fileName != NULL)
    {
        char *path = getFullPath(acqCon.savePath, acqCon.fileName);
        deleteFile(path);
        free(path);
    }
}

//3D 폴더 삭제
void deleteAcq3DFolder()
{
    cJSON *list = cJSON_CreateArray();
    getDirList(list, acqCon.savePath, true);
    deleteDirList(list);
    deleteDir(acqCon.savePath); //자기 자신 삭제
    cJSON_Delete(list);
}

//취득 종료
void stopAcq()
{
    endFileWrite();
    acqCon.runAcq = false;
}

//취득 시작
void startAcq()
{
    //파일이 존재하지 않으면 파일 생성
    if (acqCon.savePath != NULL)
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
}

//노벨다칩에서 발생한 펄스 데이터 저장
void frontRowData()
{
    //취득 시간 구함. 취득 시간을 앱에 전송해서 앱에서 취득속도를 체크하기 위함
    struct timeval time;
    gettimeofday(&time, NULL);
    double start_ms = (double)time.tv_sec * 1000000 + (double)time.tv_usec;
    memcpy(acqCon.NVA_readData + fixDepthDataSize, &start_ms, sizeof(start_ms));

    if (acqCon.fp != NULL)
    {
        //취득 데이터를 파일에 씀
        if (fwrite(acqCon.NVA_readData, 1, fixDepthDataSize, acqCon.fp) == fixDepthDataSize)
        {
            acqCon.dataCnt++;
            //취득 데이터 앱에 전송
            socket_write(ACQ_DATA_NTF, acqCon.NVA_readData, sizeof(acqCon.NVA_readData));
        }
    }
}

//뒤로 이동했을 때 처리
void backRowData()
{
    if (acqCon.fp != NULL)
    {
        //파일 쓰는 위치를 펄스 데이터 크기인 640byte 만큼 뒤로 이동.
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

//취득이 완료되면 헤더정보을 포함하여 저장을 완료함
bool saveAcq(char *headerInfo, int size)
{
    bool saveState = false;
    char *path = getFullPath(acqCon.savePath, acqCon.fileName);
    FILE *fp = fopen(path, "r+");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_SET);
        fwrite(headerInfo, 1, size, fp);
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

//모바일에서 받은 시간으로 라즈베리 시간을 세팅함
void acqDateTime(char *dateTime, int size)
{
    char timeStr[size], buf[200];
    strncpy(timeStr, dateTime, size);
    sprintf(buf, "sudo timedatectl set-time '%s'", timeStr);
    pclose(popen("sudo timedatectl set-ntp false", "r"));
    pclose(popen(buf, "r"));
}

//앱에서 취득화면 진입
void acqOn()
{
    //레이저 킴
    digitalWrite(LASER_PIN, HIGH);
    //엔코더 킴
    digitalWrite(ENCODER_POWER_PIN, HIGH);
    //노벨다칩 초기화
    NVA_Init(0);
    if (NVAParam.ChipID != 0x0306)
    {
        printf("Novelda initialization failed\n");
    }
}

//앱에서 취득화면 나감
void acqOff()
{
    digitalWrite(LASER_PIN, LOW);
    digitalWrite(ENCODER_POWER_PIN, LOW);
}
