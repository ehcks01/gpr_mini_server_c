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

char *strRealPath;

bool initRealPath()
{
    char pathBuf[4096];
    char *res = realpath(".", pathBuf);
    if (!res)
    {
        return false;
    }
    else
    {
        strRealPath = malloc(strlen(pathBuf));
        strcpy(strRealPath, pathBuf);
        return true;
    }
}

char *getFullPath(const char *path, const char *filename)
{
    char *fullPath = calloc(strlen(path) + strlen(filename) + 2, 1);
    strcpy(fullPath, path);
    strcat(fullPath, "/");
    strcat(fullPath, filename);
    return fullPath;
}

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

        do {
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

void deleteDirList(cJSON *list)
{
    for (int i = cJSON_GetArraySize(list) - 1; i >= 0; i--)
    {
        cJSON *subitem = cJSON_GetArrayItem(list, i);
        char *tempPath = cJSON_GetObjectItem(subitem, "path")->valuestring;

        if (cJSON_IsTrue(cJSON_GetObjectItem(subitem, "isDir")))
        {
            deleteDir(tempPath);
        }
        else
        {
            deleteFile(tempPath);
        }
    }
}

void addDirInfo(cJSON *root, char *path)
{
    struct stat stat_path;
    size_t path_len;

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

void getDirList(cJSON *root, char *path, bool repet)
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_path, stat_entry;
    size_t path_len;

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

    // the length of the path
    path_len = strlen(path);

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

        cJSON *file = cJSON_CreateObject();
        cJSON_AddItemToArray(root, file);
        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0)
        {
            cJSON_AddTrueToObject(file, "isDir");
            if (repet)
            {
                getDirList(root, full_path, repet);
            }
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

char *getDiskSize(char *path, bool isUsb)
{
    FILE *f;
    char *buf;
    cJSON *root;

    char *command = "df -h";
    f = popen(command, "r");
    if (f == NULL)
    {
        return NULL;
    }
    root = cJSON_CreateObject();
    buf = malloc(1024);
    // Filesystem      Size  Used Avail Use% Mounted on
    // /dev/sda1        15G   40K   15G   1% /home/gpr_mini_zero/usb
    while (fgets(buf, 1024, f) != NULL)
    {
        char *ptr = strstr(buf, path);
        if (ptr != NULL)
        {
            ptr = strtok(buf, " ");
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
            break;
        }
    }

    if (isUsb)
    {
        char *model = getUsbDiskModel();
        cJSON_AddItemToObject(root, "model", cJSON_CreateString(model));
        free(model);
    }

    char *out = cJSON_Print(root);

    cJSON_Delete(root);
    pclose(f);
    free(buf);

    return out;
}