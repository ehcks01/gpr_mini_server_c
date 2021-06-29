#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "gpr_socket.h"
#include "gpr_socket_data.h"
#include "gpr_socket_protocol.h"
#include "gpr_socket_acq.h"
#include "gpr_socket_ana.h"
#include "../NVA/NVA_CON.h"
#include "../NVA/NVA_file.h"
#include "../common/gpr_param.h"
#include "../common/dir_control.h"
#include "../common/usb_control.h"

int server_socket, client_socket;

bool socket_ready()
{
    struct sockaddr_in server_addr;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (-1 == server_socket)
    {
        printf("server socket failed\n");
        return false;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (-1 == bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        printf("server bind failed\n");
        return false;
    }

    if (-1 == listen(server_socket, 5))
    {
        printf("server listen failed\n");
        return false;
    }

    printf("Server listening..\n");
    return true;
}

int socket_client_accept()
{
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);

    if (-1 == client_socket)
    {
        printf("client accept failed\n");
        return false;
    }
    char *str = inet_ntoa(client_addr.sin_addr);
    printf("client connection: %s\n", str);
    return true;
}

int socket_receive(char *buff_rcv)
{
    return read(client_socket, buff_rcv, sizeof(buff_rcv));
}

void socket_client_done()
{
    printf("connection close\n");
    close(client_socket);
}

void socket_server_done()
{
    close(server_socket);
}

void socket_write(char code, void *bytes, int size)
{
    int sendLen = size + 1;
    char *send = malloc(sendLen + 4);
    intToBytes(sendLen, send, 4);
    *(send + 4) = code;
    memcpy(send + 5, bytes, size);

    write(client_socket, send, sendLen + 4);
    free(send);
}

void sendSavePath()
{
    char *str = jsonOfsendSavePath();
    socket_write(ACQ_SAVE_PATH_NTF, str, strlen(str));
    free(str);
}

void socket_read(unsigned char buffer[], int buff_size)
{
    // 받은 버퍼를 이벤트로 변환
    convertEvent(buffer, buff_size);
    if (tcpData.event_list_cnt > 0)
    {
        for (int i = 0; i < tcpData.event_list_cnt; i++)
        {
            switch (tcpData.event_list[i][0])
            {
            case HEADER_INFO_FTN:
                setHeaderFromJson(*(tcpData.event_list + i) + 1);
                break;
            case ACQ_LASER_ON_FTN:
                laserOn();
                break;
            case ACQ_INFO_FTN:
                setAcqInfoFromJson(*(tcpData.event_list + i) + 1);

                if (strlen(acqCon.savePath) == 0)
                {
                    makeSavePath();
                }
                sendSavePath();
                break;
            case ACQ_START_FTN:
                startAcq();
                socket_write(ACQ_START_NTF, "", 0);
                break;
            case ACQ_STOP_FTN:
                stopAcq();
                socket_write(ACQ_STOP_NTF, "", 0);
                break;
            case ACQ_NON_SAVE_FTN:
                deleteAcqFile();
                usleep(1);
                break;
            case ACQ_SAVE_FTN:
                saveAcq(*(tcpData.event_list + i) + 1, tcpData.event_length_list[i]);
                socket_write(ACQ_SAVE_NTF, "", 0);

                // 2d일땐 다음 파일 준비
                if (headerParameter.cScanMode == 0)
                {
                    makeSavePath();
                    sendSavePath();
                }
                usleep(1);
                break;
            case ACQ_REFRESH_FTN:
                endFileWrite();
                deleteAcqFile();

                socket_write(ACQ_REFRESH_NTF, "", 0);

                if (acqCon.runAcq)
                {
                    startAcq();
                }
                break;
            case ACQ_ABNORMAL_QUIT:
                stopAcq();
                if (is2DScanMode())
                {
                    deleteAcqFile(); //파일 삭제
                }
                else
                {
                    deleteAcq3DFolder();
                }
                break;
            case NVA_REQUEST_FTN:
            {
                char *json = getNVAJson();
                socket_write(NVA_RESPONSE_NTF, json, strlen(json));
                free(json);
                break;
            }
            case NVA_MODIFY_FTN:
                setNVASetting(*(tcpData.event_list + i) + 1);
                saveNVASetting();
                socket_write(NVA_COMPLETE_NTF, "", 0);
                GPR_Init(0);
                break;
            case ANA_ROOT_DIR_FTN:
                sendRootDir();
                break;
            case ANA_DISK_SIZE_FTN:
                sendDiskSize();
                break;
            case ANA_READ_DIR_FTN:
                sendReadDir(*(tcpData.event_list + i) + 1, ANA_READ_DIR_NTF, false);
                break;
            case ANA_CHECK_DIR_FTN:
                sendReadDir(*(tcpData.event_list + i) + 1, ANA_CHECK_DIR_NTF, true);
                break;
            case ANA_UNCHECK_DIR_FTN:
                sendReadDir(*(tcpData.event_list + i) + 1, ANA_UNCHECK_DIR_NTF, true);
                break;
            case ANA_DELETE_FILE_FTN:
                sendDeleteFile(*(tcpData.event_list + i) + 1);
                break;
            case ANA_DELETE_FOLDER_FTN:
                sendDeleteFolder(*(tcpData.event_list + i) + 1);
                break;
            case ANA_USB_INFO_FTN:
                sendUsbInFo();
                break;
            case ANA_USB_COPY_FTN:
                socket_write(ANA_USB_COPY_NTF, "", 0);
                tryCopyFiles(*(tcpData.event_list + i) + 1);
                socket_write(ANA_USB_COPY_DONE_NTF, "", 0);
                break;
            case ANA_LOAD_FILE_FTN:
                sendFileData(*(tcpData.event_list + i) + 1);
                socket_write(ANA_LOAD_FILE_NTF, "", 0);
                break;
            case ANA_LOAD_FILE_WITH_CONFIG_FTN:
                sendFileData(*(tcpData.event_list + i) + 1);
                socket_write(ANA_LOAD_FILE_WITH_CONFIG_NTF, "", 0);
                break;
            case ANA_LOAD_CONFIG_FILE_FTN:
                sendLoadConfiFile(*(tcpData.event_list + i) + 1);
                break;
            case ANA_SAVE_CONFIG_FILE_FTN:
                sendSaveConfigFile(*(tcpData.event_list + i) + 1);
                break;
            case ANA_LOAD_TOP_VIEW_FNT:
                sendFileData(*(tcpData.event_list + i) + 1);
                socket_write(ANA_LOAD_TOP_VIEW_NTF, "", 0);
                break;
            default:
                break;
            }

            free(*(tcpData.event_list + i));
        }
        tcpData.event_list_cnt = 0;
    }
}

void socket_close()
{
    stopAcq();
    laserOff();
    // printf("close :%d \n", tcpData.total_length);
}
