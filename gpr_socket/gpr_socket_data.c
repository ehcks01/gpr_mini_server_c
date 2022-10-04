#include <stdlib.h>
#include <string.h>

#include "gpr_socket.h"
#include "gpr_socket_data.h"
#include "gpr_socket_acq.h"
#include "../common/gpr_param.h"
#include "../common/cJSON.h"

struct TcpData tcpData = {.index = 0, .length = 0, .checkSum = 0, .data_buffer = NULL};

int bytesToInt(char *buffer, int size)
{
    // little endian
    int integer = 0;
    for (int i = 0; i < size; i++)
    {
        integer += buffer[i] << i * 8;
    }
    return integer;
}

void intToBytes(int integer, char *buffer, int size)
{
    // little endian
    for (int i = 0; i < size; i++)
    {
        buffer[i] = (integer >> i * 8);
    }
}

void convertEvent(char buffer[], int buffer_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        if (tcpData.index == 0)
        {
            if (buffer[i] == 0x7E)
            {
                tcpData.index++;
            }
            continue;
        }

        // length 1-2 (2byte)
        if (tcpData.index == 1)
        {
            tcpData.length = buffer[i];
        }
        else if (tcpData.index == 2)
        {
            tcpData.length = (tcpData.length << 8) + buffer[i];
            tcpData.data_buffer = (char *)calloc(tcpData.length, 1);
        }
        // frameType
        else if (tcpData.index == 3)
        {
            tcpData.checkSum += buffer[i];
            tcpData.data_buffer[0] = buffer[i];
        }
        else if (tcpData.index < tcpData.length + 3)
        {
            tcpData.checkSum += buffer[i];
            tcpData.data_buffer[tcpData.index - 3] = buffer[i];
        }
        else
        {
            tcpData.checkSum += buffer[i];
            if ((tcpData.checkSum & 0xFF) == 0xFF)
            {
                socket_read(tcpData.data_buffer, tcpData.length);
            }
            //초기화
            tcpData.checkSum = 0;
            tcpData.index = -1;
            tcpData.length = 0;
            free(tcpData.data_buffer);
        }
        tcpData.index++;
    }
}

void eliminate_json(char *str)
{
    while (*str != '\0')
    {
        if (*str == '[' || *str == ']' || *str == ' ')
        {
            strcpy(str, str + 1);
            str--;
        }
        str++;
    }
}

//메모리 해제 필요
char *arrayCodeToStr(char *arrayCode)
{
    char temp_array[strlen(arrayCode)];
    strcpy(temp_array, arrayCode);
    char *token = strtok(temp_array, ",");
    int token_index = 0;
    while (token != NULL)
    {
        token_index++;
        token = strtok(NULL, ",");
    }

    char *str = (char *)calloc(token_index, 1);
    token = strtok(arrayCode, ",");
    token_index = 0;
    while (token != NULL)
    {
        eliminate_json(token);
        str[token_index] = atoi(token);
        token_index++;
        token = strtok(NULL, ",");
    }
    return str;
}

void setHeaderFromJson(char *bytes)
{
    cJSON *json = cJSON_Parse(bytes);
    if (json != NULL)
    {
        char *str = cJSON_GetObjectItem(json, "strDate")->valuestring;
        memset(headerParameter.strDate, 0, strlen(headerParameter.strDate));
        memcpy(headerParameter.strDate, str, strlen(str));
        // printf("strData: %s\n", headerParameter.strDate);

        headerParameter.cResolution = cJSON_GetObjectItem(json, "cResolution")->valueint;
        // printf("cResolution: %d\n", headerParameter.cResolution);

        headerParameter.sLength = cJSON_GetObjectItem(json, "sLength")->valueint;
        // printf("sLength: %d\n", headerParameter.sLength);

        headerParameter.cScanMode = cJSON_GetObjectItem(json, "cScanMode")->valueint;
        // printf("cScanMode: %d\n", headerParameter.cScanMode);

        headerParameter.cDepth = cJSON_GetObjectItem(json, "cDepth")->valueint;
        // printf("cDepth: %d\n", headerParameter.cDepth);

        headerParameter.cUnit = cJSON_GetObjectItem(json, "cUnit")->valueint;
        // printf("cUnit: %d\n", headerParameter.cUnit);

        headerParameter.fDielectric = cJSON_GetObjectItem(json, "fDielectric")->valuedouble;
        // printf("fDielectric: %f\n", headerParameter.fDielectric);

        str = cJSON_GetObjectItem(json, "strSiteName")->valuestring;
        str = arrayCodeToStr(str);
        memset(headerParameter.strSiteName, 0, strlen(headerParameter.strSiteName));
        memcpy(headerParameter.strSiteName, str, strlen(str));
        free(str);
        // printf("strSiteName: %s\n", headerParameter.strSiteName);

        str = cJSON_GetObjectItem(json, "strOperator")->valuestring;
        str = arrayCodeToStr(str);
        memset(headerParameter.strOperator, 0, strlen(headerParameter.strOperator));
        memcpy(headerParameter.strOperator, str, strlen(str));
        free(str);
        // printf("strOperator: %s\n", headerParameter.strOperator);

        headerParameter.cCoordinate = cJSON_GetObjectItem(json, "cCoordinate")->valueint;
        // printf("cCoordinate: %d\n", headerParameter.cCoordinate);

        headerParameter.cBlowNo = cJSON_GetObjectItem(json, "cBlowNo")->valueint;
        // printf("cBlowNo: %d\n", headerParameter.cBlowNo);

        headerParameter.cSaveMode = cJSON_GetObjectItem(json, "cSaveMode")->valueint;
        // printf("cSaveMode: %d\n", headerParameter.cSaveMode);

        headerParameter.sLineCount = cJSON_GetObjectItem(json, "sLineCount")->valueint;
        // printf("sLineCount: %d\n", headerParameter.sLineCount);

        headerParameter.cGainSW = cJSON_GetObjectItem(json, "cGainSW")->valueint;
        // printf("cGainSW: %d\n", headerParameter.cGainSW);

        headerParameter.cExpGain = cJSON_GetObjectItem(json, "cExpGain")->valueint;
        // printf("cExpGain: %d\n", headerParameter.cExpGain);

        headerParameter.sHPFilter = cJSON_GetObjectItem(json, "sHPFilter")->valueint;
        // printf("sHPFilter: %d\n", headerParameter.sHPFilter);

        headerParameter.fNanoTime = cJSON_GetObjectItem(json, "fNanoTime")->valuedouble;
        // printf("fNanoTime: %f\n", headerParameter.fNanoTime);

        headerParameter.fLineNoiseFilter = cJSON_GetObjectItem(json, "fLineNoiseFilter")->valuedouble;
        // printf("fLineNoiseFilter: %f\n", headerParameter.fLineNoiseFilter);

        headerParameter.sLPFilter = cJSON_GetObjectItem(json, "sLPFilter")->valueint;
        // printf("sLPFilter: %d\n", headerParameter.sLPFilter);

        headerParameter.cColorType = cJSON_GetObjectItem(json, "cColorType")->valueint;
        // printf("cColorType: %d\n", headerParameter.cColorType);

        cJSON_Delete(json);
    }
}

void setAcqInfoFromJson(char *bytes)
{
    cJSON *json = cJSON_Parse(bytes);
    if (json != NULL)
    {
        char *str = cJSON_GetObjectItem(json, "fileName")->valuestring;
        str = arrayCodeToStr(str);
        free(acqCon.fileName);
        acqCon.fileName = (char *)calloc(strlen(str), 1);
        memcpy(acqCon.fileName, str, strlen(str));
        printf("acqCon.fileName: %s\n", acqCon.fileName);

        str = cJSON_GetObjectItem(json, "savePath")->valuestring;
        str = arrayCodeToStr(str);
        free(acqCon.savePath);
        acqCon.savePath = (char *)calloc(strlen(str), 1);
        memcpy(acqCon.savePath, str, strlen(str));
        printf("acqCon.savePath: %s\n", acqCon.savePath);

        acqCon.bForwardScan = cJSON_GetObjectItem(json, "scanDirection")->valueint;
        printf("scanDirection: %d\n", acqCon.bForwardScan);

        if (headerParameter.cScanMode != 0)
        {
            acqCon.dataSize3D = cJSON_GetObjectItem(json, "dataSize3D")->valueint;
            printf("acqCon.dataSize3D: %d\n", acqCon.dataSize3D);

            str = cJSON_GetObjectItem(json, "grid3D")->valuestring;
            free(acqCon.grid3D);
            acqCon.grid3D = (char *)calloc(strlen(str), 1);
            memcpy(acqCon.grid3D, str, strlen(str));
            printf("acqCon.grid3D: %s\n", acqCon.grid3D);
        }

        cJSON_Delete(json);
    }
}

char *jsonOfsendSavePath()
{
    cJSON *root;
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "fileName", cJSON_CreateString(acqCon.fileName));
    cJSON_AddItemToObject(root, "savePath", cJSON_CreateString(acqCon.savePath));
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    return out;
}