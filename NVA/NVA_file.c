#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../cJSON/cJSON.h"
#include "NVA6100.h"
#include "NVA_file.h"

bool getNVAPath(char path[])
{
    char *res = realpath(".", path);
    if (res)
    {
        strcat(path, "/NVA_Setting");
        return true;
    }
    return false;
}

char *getNVAJson()
{
    cJSON *root;
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "Gain", cJSON_CreateNumber(NVAParam.Gain));
    cJSON_AddItemToObject(root, "Iterations", cJSON_CreateNumber(NVAParam.Iterations));
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

void saveNVASetting()
{
    char *out = getNVAJson();

    char path[1024];
    if (getNVAPath(path))
    {
        FILE *fp = fopen(path, "w");
        if (fp != NULL)
        {
            fwrite(out, 1, strlen(out), fp);
            fclose(fp);
        }
    }
    free(out);
}

char *readNVAFile()
{
    char path[1024];
    if (getNVAPath(path))
    {
        FILE *fp = fopen(path, "r");
        if (fp != NULL)
        {
            fseek(fp, 0, SEEK_END);
            long lSize = ftell(fp);
            rewind(fp);

            char *buffer = (char *)malloc(sizeof(char) * lSize);
            size_t result = fread(buffer, 1, lSize, fp);
            if (result != lSize)
            {
                printf("Reading error: %s \n", path);
                return NULL;
            }

            fclose(fp);
            return buffer;
        }
    }
    return NULL;
}

void setNVASetting(char *bytes)
{
    cJSON *json = cJSON_Parse(bytes);
    if (json != NULL)
    {
        NVAParam.Gain = cJSON_GetObjectItem(json, "Gain")->valueint;
        NVAParam.Iterations = cJSON_GetObjectItem(json, "Iterations")->valueint;
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

void loadNVASetting()
{
    char *bytes = readNVAFile();
    if (bytes != NULL)
    {
        setNVASetting(bytes);
        free(bytes);
    }
}

