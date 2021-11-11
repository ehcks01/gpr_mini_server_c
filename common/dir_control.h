#ifndef dir_control__h
#define dir_control__h

#include <stdbool.h>
#include "cJSON.h"

char *getFullPath(const char *path, const char *filename);
void mkdirs(char *dir);
bool copyFile(const char *to, const char *from);
bool deleteFile(char *path);
bool deleteDir(char *path);
void deleteDirList(cJSON *list);
void addDirInfo(cJSON *root, char *path);
void getDirList(cJSON* root, char *path, bool repet);
char *getFileNameFromPath(char *path);
cJSON *getDiskSize(char *path);
bool initRealPath(char *argv0);

//mgm파일이 header info가 전부 0이면 저장이 잘못된 파일이라 보고 삭제.
bool checkHeaderInfoFromAcqFile(char *path);

extern char strRealPath[100];
extern char strExeName[100];

#endif