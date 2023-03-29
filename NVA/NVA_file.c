#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/cJSON.h"
#include "../common/dir_control.h"
#include "NVA6100.h"
#include "NVA_file.h"

char strNVAPath[100];

//노벨다칩 세팅 정보를 라즈베리에 저장하는 경로
bool initNVAPath()
{
    char *NVAPath = "/NVA_Setting";
    strcpy(strNVAPath, strRealPath);
    strcat(strNVAPath, NVAPath);

    return true;
}

//노발다칩 세팅 정보를 json형태로 바꿈
char *getNVAJson()
{
    cJSON *root;
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "Gain", cJSON_CreateNumber(NVAParam.Gain));
    // cJSON_AddItemToObject(root, "Iterations", cJSON_CreateNumber(NVAParam.Iterations));
    cJSON_AddItemToObject(root, "CoarseTune", cJSON_CreateNumber(NVAParam.CoarseTune));
    cJSON_AddItemToObject(root, "MediumTune", cJSON_CreateNumber(NVAParam.MediumTune));
    cJSON_AddItemToObject(root, "FineTune", cJSON_CreateNumber(NVAParam.FineTune));
    // cJSON_AddItemToObject(root, "SamplingCount", cJSON_CreateNumber(NVAParam.SamplingCount));
    // cJSON_AddItemToObject(root, "SamplingRate", cJSON_CreateNumber(NVAParam.SamplingRate));
    // cJSON_AddItemToObject(root, "DACMin", cJSON_CreateNumber(NVAParam.DACMin));
    // cJSON_AddItemToObject(root, "DACMax", cJSON_CreateNumber(NVAParam.DACMax));
    // cJSON_AddItemToObject(root, "DACStep", cJSON_CreateNumber(NVAParam.DACStep));
    // cJSON_AddItemToObject(root, "CounterBitSelOut", cJSON_CreateNumber(NVAParam.CounterBitSelOut));
    // cJSON_AddItemToObject(root, "SamplingRateMeasResult", cJSON_CreateNumber(NVAParam.SamplingRateMeasResult));
    // cJSON_AddItemToObject(root, "SweepControllerStatus", cJSON_CreateNumber(NVAParam.SweepControllerStatus));
    // cJSON_AddItemToObject(root, "TimingMeasResult", cJSON_CreateNumber(NVAParam.TimingMeasResult));
    // cJSON_AddItemToObject(root, "FocusMax", cJSON_CreateNumber(NVAParam.FocusMax));
    // cJSON_AddItemToObject(root, "FocusMin", cJSON_CreateNumber(NVAParam.FocusMin));

    char *out = cJSON_Print(root);
    cJSON_Delete(root);

    return out;
}

//노벨다칩 세팅 정보를 라즈베리에 파일로 저장
void saveNVASetting()
{
    char *out = getNVAJson();
    FILE *fp = fopen(strNVAPath, "w");
    if (fp != NULL)
    {
        fwrite(out, 1, strlen(out), fp);
        fclose(fp);
    }
    free(out);
}

//노벨다칩 세팅 정보를 라즈베리에 저장된 파일에서 읽음
char *readNVAFile()
{
    FILE *fp = fopen(strNVAPath, "r");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        long lSize = ftell(fp);
        rewind(fp);

        char *buffer = (char *)malloc(sizeof(char) * lSize);
        size_t result = fread(buffer, 1, lSize, fp);
        if (result != lSize)
        {
            printf("Reading error: %s \n", strNVAPath);
            return NULL;
        }

        fclose(fp);
        return buffer;
    }
    return NULL;
}

//json 형태로 받은 노벨다칩 세팅정보를 적용
void setNVASetting(char *bytes)
{
    cJSON *json = cJSON_Parse(bytes);
    //Gain, CoarseTune, MediumTune, FineTune 정보만 세팅하도록 했음
    if (json != NULL)
    {
        NVAParam.Gain = cJSON_GetObjectItem(json, "Gain")->valueint;
        // NVAParam.Iterations = cJSON_GetObjectItem(json, "Iterations")->valueint;
        NVAParam.CoarseTune = cJSON_GetObjectItem(json, "CoarseTune")->valueint;
        NVAParam.MediumTune = cJSON_GetObjectItem(json, "MediumTune")->valueint;
        NVAParam.FineTune = cJSON_GetObjectItem(json, "FineTune")->valueint;
        // NVAParam.SamplingCount = cJSON_GetObjectItem(json, "SamplingCount")->valueint;
        // NVAParam.SamplingRate = cJSON_GetObjectItem(json, "SamplingRate")->valueint;
        // NVAParam.DACMin = cJSON_GetObjectItem(json, "DACMin")->valueint;
        // NVAParam.DACMax = cJSON_GetObjectItem(json, "DACMax")->valueint;
        // NVAParam.DACStep = cJSON_GetObjectItem(json, "DACStep")->valueint;
        // NVAParam.CounterBitSelOut = cJSON_GetObjectItem(json, "CounterBitSelOut")->valueint;
        // NVAParam.SamplingRateMeasResult = cJSON_GetObjectItem(json, "SamplingRateMeasResult")->valueint;
        // NVAParam.SweepControllerStatus = cJSON_GetObjectItem(json, "SweepControllerStatus")->valueint;
        // NVAParam.TimingMeasResult = cJSON_GetObjectItem(json, "TimingMeasResult")->valueint;
        // NVAParam.FocusMax = cJSON_GetObjectItem(json, "FocusMax")->valueint;
        // NVAParam.FocusMin = cJSON_GetObjectItem(json, "FocusMin")->valueint;

        cJSON_Delete(json);
    }
}

//벨다칩 세팅정보를 파일에서 읽고 적용하는 부분을 묶은 함수
void loadNVASetting()
{
    //노벨다 칩정보를 txt에서 읽어옴
    char *bytes = readNVAFile();

    if (bytes != NULL)
    {
        setNVASetting(bytes);
        free(bytes);
    }
}
