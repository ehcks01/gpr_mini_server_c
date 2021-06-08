#ifndef gpr_socket_acq__h
#define gpr_socket_acq__h

#include <stdbool.h>
#include <stdio.h>

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

extern struct AcqContoroller acqCon;

bool is2DScanMode();
bool isNotFull3DData();
char *getFullPath(const char *path, const char *filename);
bool fileexists(const char *path, const char *filename);
bool makeSavePath();
void endFileWrite();
void deleteAcqFile();
void deleteAcq3DFolder();
void stopAcq();
void _mkdir(const char *dir);
void startAcq();
void frontRowData();
void backRowData();
void saveAcq(char *headerInfo, int size);

#endif