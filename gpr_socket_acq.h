#include <sys/stat.h>

void socket_write(char code, void *bytes, int size);

struct AcqContoroller
{
    FILE *fp;
    char *fileName;
    char *savePath;
    int dataCnt;                     //취득 중인 데이터 총량
    int dataSize3D;                  //취득해야 하는 데이터 사이즈
    char *grid3D;                    //그리드 2x2, 1x1 표시
    bool runAcq;                     //취득 데이타를 전송 중인지
    unsigned char NVA_readData[640]; //취득한 데이타
};

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

char *getFullPath(const char *path, const char *filename)
{
    char *name = malloc(strlen(path) + strlen(filename) + 1);
    strcpy(name, path);
    strcat(name, "/");
    strcat(name, filename);
    return name;
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
    char *res = realpath(".", pathBuf);

    if (!res)
    {
        return false;
    }

    int inc = 0;
    strcat(pathBuf, "/data/");
    strcat(pathBuf, headerParameter.strSiteName);

    if (is2DScanMode())
    {
        strcat(pathBuf, "/2D");
        free(acqCon.savePath);
        acqCon.savePath = (char *)calloc(strlen(pathBuf), 1);
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
    char *path = getFullPath(acqCon.savePath, acqCon.fileName);
    int nResult = remove(path);
    if (nResult == 0)
    {
        printf("file delete successed: %s\n", path);
    }
    else if (nResult == -1)
    {
        printf("file delete failed: %s\n", path);
    }
    free(path);
}

void deleteAcq3DFolder()
{
    int nResult = remove(acqCon.savePath);
    if (nResult == 0)
    {
        printf("folder delete successed: %s\n", acqCon.savePath);
    }
    else if (nResult == -1)
    {
        printf("folder delete failed: %s\n", acqCon.savePath);
    }
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
        mkdir(acqCon.savePath, 0700);
    }

    if (fileexists(acqCon.savePath, acqCon.fileName))
    {
        deleteAcqFile();
    }
    acqCon.dataCnt = 0;
    char *path = getFullPath(acqCon.savePath, acqCon.fileName);
    acqCon.fp = fopen(path, "wb");
    fseek(acqCon.fp, fixHeaderSize, SEEK_SET);
    free(path);
    acqCon.runAcq = true;
}

void frontRowData()
{
    acqCon.dataCnt++;
    fwrite(acqCon.NVA_readData, 1, sizeof(acqCon.NVA_readData), acqCon.fp);
    socket_write(ACQ_DATA_NTF, acqCon.NVA_readData, sizeof(acqCon.NVA_readData));
}

void backRowData()
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

void saveAcq(char *headerInfo, int size)
{
    char *path = getFullPath(acqCon.savePath, acqCon.fileName);
    FILE *fp = fopen(path, "r+");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_SET);
        //앞에 1바이트는 socket code라서 제외 시킴
        fwrite(headerInfo + 1, 1, size - 1, fp);
        fclose(fp);
        printf("file save successed: %s \n", path);
    }
    else
    {
        printf("file save failed: %s \n", path);
    }
    free(path);
}