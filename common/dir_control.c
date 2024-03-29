#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "dir_control.h"
#include "cJSON.h"
#include "usb_control.h"

char strExePath[100]; //실행 파일 경로
char strExeName[100];  //실행 파일 이름

//프로그램의 이름과 실행경로를 구함
bool initRealPath(char *argv0)
{
    //프로그램 전체 경로 구함
    char *res = realpath(argv0, strExePath);

    if (!res)
    {
        return false;
    }
    else
    {
        char copyStr[100];
        strcpy(copyStr, res);
        char *ptr = strtok(res, "/");
        char *lastPtr = ptr;
        while ((ptr = strtok(NULL, "/")) != NULL)
        {
            lastPtr = ptr;
        }
        size_t reSize = strlen(copyStr) - strlen(lastPtr) - 1;
        memset(res, 0, sizeof(res));
        //프로그램 실행경로
        strncpy(res, copyStr, reSize);
        //프로그램 이름
        strcpy(strExeName, lastPtr);
        return true;
    }
}

//경로와 파일이름을 합쳐서 반환
char *getFullPath(const char *path, const char *filename)
{
    char *fullPath = calloc(strlen(path) + strlen(filename) + 2, 1);
    strcpy(fullPath, path);
    strcat(fullPath, "/");
    strcat(fullPath, filename);
    return fullPath;
}

//라즈베리 OS에 경로를 폴더로 생성
void mkdirs(char *dir)
{
    char tmp[2048];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
}

//라즈베리 OS에서 파일을 복사
bool copyFile(const char *to, const char *from)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return false;

    fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do
        {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return true;
    }

out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return false;
}


//라즈베리 OS에서 파일을 삭제
bool deleteFile(char *path)
{
    if (unlink(path) == 0)
    {
        printf("Removed a file: %s\n", path);
        return true;
    }
    else
    {
        printf("Can`t remove a file: %s\n", path);
        return false;
    }
}

//라즈베리 OS에서 폴더를 삭제
bool deleteDir(char *path)
{
    if (rmdir(path) == 0)
    {
        printf("Removed a directory: %s\n", path);
        return true;
    }
    else
    {
        printf("Can`t remove a directory: %s\n", path);
        return false;
    }
}

//라즈베리 OS에서 경로들을 삭제
void deleteDirList(cJSON *list)
{
    //먼저 파일을 삭제함 - 파일이 있으면 폴더 삭제가 안됨
    for (int i = cJSON_GetArraySize(list) - 1; i >= 0; i--)
    {
        cJSON *subitem = cJSON_GetArrayItem(list, i);
        char *tempPath = cJSON_GetObjectItem(subitem, "path")->valuestring;

        if (!cJSON_IsTrue(cJSON_GetObjectItem(subitem, "isDir")))
        {
           deleteFile(tempPath);
        }
    }

    //폴더를 삭제함. 한번씩 삭제가 여기서 문제가 있는지 확인
    for (int i = cJSON_GetArraySize(list) - 1; i >= 0; i--)
    {
        cJSON *subitem = cJSON_GetArrayItem(list, i);
        char *tempPath = cJSON_GetObjectItem(subitem, "path")->valuestring;

        if (cJSON_IsTrue(cJSON_GetObjectItem(subitem, "isDir")))
        {
            deleteDir(tempPath);
        }
    }
}

//해당 경로 정보를 json 형태로 변환.
void addDirInfo(cJSON *root, char *path)
{
    struct stat stat_path;

    if (root == NULL)
    {
        printf("cJSON root not initialization\n");
        return;
    }
    if (stat(path, &stat_path) < 0)
    {
        fprintf(stderr, "%s: %s\n", "Unable to get file properties.", path);
        return;
    }
    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0)
    {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        return;
    }

    cJSON *file = cJSON_CreateObject();
    cJSON_AddItemToArray(root, file);
    cJSON_AddTrueToObject(file, "isDir");
    cJSON_AddStringToObject(file, "path", path);
    cJSON_AddStringToObject(file, "name", getFileNameFromPath(path));
    cJSON_AddStringToObject(file, "date", ctime(&stat_path.st_atime));
    cJSON_AddNumberToObject(file, "size", stat_path.st_size);
}

//해당 경로 및 하위 경로정보 까지 json 형태로 변환.
void getDirList(cJSON *root, char *path, bool repet)
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_path, stat_entry;

    if (root == NULL)
    {
        printf("cJSON root not initialization\n");
        return;
    }
    if (stat(path, &stat_path) < 0)
    {
        fprintf(stderr, "%s: %s\n", "Unable to get file properties.", path);
        return;
    }
    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0)
    {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        return;
    }
    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL)
    {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }

        char *full_path = getFullPath(path, entry->d_name);
        // stat for the entry
        if (stat(full_path, &stat_entry) < 0)
        {
            free(full_path);
            continue;
        }

        bool isDir = true;
        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0)
        {
            if (repet)
            {
                getDirList(root, full_path, repet);
            }
        }
        else
        {
            if (checkHeaderInfoFromAcqFile(full_path) == false)
            {
                free(full_path);
                continue;
            }
            isDir = false;
        }
        cJSON *file = cJSON_CreateObject();
        cJSON_AddItemToArray(root, file);
        if (isDir)
        {
            cJSON_AddTrueToObject(file, "isDir");
        }
        else
        {
            cJSON_AddFalseToObject(file, "isDir");
        }
        cJSON_AddStringToObject(file, "path", full_path);
        cJSON_AddStringToObject(file, "name", entry->d_name);
        cJSON_AddStringToObject(file, "date", ctime(&stat_entry.st_atime));
        cJSON_AddNumberToObject(file, "size", stat_entry.st_size);
        free(full_path);
    }
    closedir(dir);
}

//파일 형태가 올바른지 256byte 헤더 정보를 확인
bool checkHeaderInfoFromAcqFile(char *path)
{
    if (strstr(path, ".MGM") == NULL)
    {
        return true;
    }
    else
    {
        int haedrSize = 256;
        char buf[haedrSize];
        int fd = open(path, O_RDONLY);

        ssize_t size = read(fd, buf, sizeof buf);
        if (size == haedrSize)
        {
            for (int i = 0; i < haedrSize; i++)
            {
                if (buf[i] != 0x00)
                {
                    return true;
                }
            }
        }
        deleteFile(path);
        return false;
    }
}

//전체 경로에서 파일이름 추출
char *getFileNameFromPath(char *path)
{
    if (path == NULL)
        return NULL;

    char *pFileName = path;
    for (char *pCur = path; *pCur != '\0'; pCur++)
    {
        if (*pCur == '/' || *pCur == '\\')
            pFileName = pCur + 1;
    }

    return pFileName;
}

//라즈베리 OS 또는 마운트한 usb의 디스크 사이즈 구하기
cJSON *getDiskSize(char *path)
{
    FILE *file;
    char buf[1024];
    cJSON *root = NULL;

    char command[50];
    strcpy(command, "df -hT ");
    strcat(command, path);
    strcat(command, " | grep ");
    strcat(command, path);
    file = popen(command, "r");
    if (file == NULL)
    {
        return NULL;
    }
    // /dev/root      ext4   29G  1.8G   27G   7% /
    if (fgets(buf, 1024, file) != NULL)
    {
        root = cJSON_CreateObject();
        char *ptr = strtok(buf, " ");
        ptr = strtok(NULL, " ");
        ptr = strtok(NULL, " ");
        if (ptr != NULL)
        {
            cJSON_AddItemToObject(root, "total", cJSON_CreateString(ptr));
            ptr = strtok(NULL, " ");
            if (ptr != NULL)
            {
                cJSON_AddItemToObject(root, "free", cJSON_CreateString(ptr));
            }
        }
    }
    pclose(file);
    return root;
}