#include <stdlib.h>
#include <string.h>

#include "gpr_socket.h"
#include "gpr_socket_data.h"
#include "gpr_socket_acq.h"
#include "../common/gpr_param.h"
#include "../common/cJSON.h"

//소켓통신으로 클라이언트에서 받은 버퍼 정보
struct TcpData tcpData = {.index = 0, .length = 0, .checkSum = 0, .data_buffer = NULL};

//바이트를 인트형태로 바꿈
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

//인트를 바이트 버퍼형태로 바꿈
void intToBytes(int integer, char *buffer, int size)
{
    // little endian
    for (int i = 0; i < size; i++)
    {
        buffer[i] = (integer >> i * 8);
    }
}

//클라이언트에서 받은 버퍼를 이벤트로 변환
void convertEvent(char buffer[], int buffer_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        //버퍼 인덱스가 0이면
        if (tcpData.index == 0)
        {
            //이벤트 시작은 0x7E로 정했으므로 다른 byte는 버림
            if (buffer[i] == 0x7E)
            {
                tcpData.index++;
            }
            continue;
        }

        // 버퍼 인덱스 1~2까지는 전송된 이벤트 길이, length(2byte)
        if (tcpData.index == 1)
        {
            tcpData.length = buffer[i];
        }
        else if (tcpData.index == 2)
        {
            tcpData.length = (tcpData.length << 8) + buffer[i];
            //길이 만큼 data_buffer 메모리 할당        
            tcpData.data_buffer = (char *)calloc(tcpData.length, 1);
        }
        else if (tcpData.index == 3)
        {
            //전송된 데이터가 정확한지 확인하기 위해 합산 checksum 
            tcpData.checkSum += buffer[i];
            //gpr_socekt_protocol.h에 정의한 code 값
            tcpData.data_buffer[0] = buffer[i];
        }
        else if (tcpData.index < tcpData.length + 3)
        {
            tcpData.checkSum += buffer[i];
            //첨부한 데이터 값
            tcpData.data_buffer[tcpData.index - 3] = buffer[i];
        }
        else
        {
            tcpData.checkSum += buffer[i];

            //전송이 완료되면 checkSum이 0xFF가 나오는지 확인
            if ((tcpData.checkSum & 0xFF) == 0xFF)
            {
                //이벤트 처리
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

//분류된 토큰에서 불필요한 문자인 [, ], 공백 3가지 제거
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

//json 형태의 포맷을 string 형식으로 변환
char *arrayCodeToStr(char *arrayCode)
{
    //앱에서 'KOREA'를 보내면 [75, 79, 82, 69, 65] 형태로 전송
    char temp_array[strlen(arrayCode)];
    strcpy(temp_array, arrayCode);

    // 첫번째 구분자 찾기
    char *token = strtok(temp_array, ",");
    int token_index = 0;
    while (token != NULL)
    {
        token_index++;
        //다음 구분자  찾기
        token = strtok(NULL, ",");
    }

    //분류된 토큰의 길이로 동적메모리 할당
    char *str = (char *)calloc(token_index, 1);

    //첫번째 구분자  찾기
    token = strtok(arrayCode, ",");
    token_index = 0;
    while (token != NULL)
    {
        //분류된 토큰에서 불필요한 문자인 [, ], 공백 3가지 제거
        eliminate_json(token);
        //char* 이므로 정수로 변환. 정확하지 않음..
        str[token_index] = atoi(token);
        token_index++;
        //다음 구분자 찾기
        token = strtok(NULL, ",");
    }

    //메모리 해제 필요
    return str;
}

//Header 정보를 가진 Json 포맷을 읽어서 원본 형태의 데이터로 추출
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
        printf("strSiteName: %s\n", str);
        str = arrayCodeToStr(str);
        printf("strSiteName: %s\n", str);
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

//취득 관련 정보를 가진 Json 포맷을 읽어서 원본 형태의 데이터로 추출
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

//파일이름과 저장경로를 json형태로 앱에 보내기 위해 변환
char *jsonForsendSavePath()
{
    cJSON *root;
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "fileName", cJSON_CreateString(acqCon.fileName));
    cJSON_AddItemToObject(root, "savePath", cJSON_CreateString(acqCon.savePath));
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    return out;
}