#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "gpr_socket.h"
#include "gpr_socket_data.h"
#include "gpr_socket_protocol.h"
#include "gpr_socket_acq.h"
#include "gpr_socket_ana.h"
#include "wifi_selector.h"
#include "../encoder/encoder.h"
#include "../NVA/NVA_CON.h"
#include "../NVA/NVA_file.h"
#include "../common/gpr_param.h"
#include "../common/dir_control.h"
#include "../common/usb_control.h"

int server_socket, client_socket;
bool server_restart = false;

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
    if (-1 == tcpSetKeepAlive(client_socket, 1, 1, 3, 1))
    {
        printf("client tcpSetKeepAlive failed\n");
        socket_client_done();
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

void socket_read(char buffer[], int buff_size)
{
    // 받은 버퍼를 이벤트로 변환
    convertEvent(buffer, buff_size);
    if (tcpData.event_list_cnt > 0)
    {
        for (int i = 0; i < tcpData.event_list_cnt; i++)
        {
            switch (tcpData.event_list[i][0])
            {
            case SERVER_INFO_FNT:
            {
                socket_write(SERVER_INFO_NTF, &battery_gauge, 1);
                break;
            }
            case HEADER_INFO_FTN:
                setHeaderFromJson(*(tcpData.event_list + i) + 1);
                break;
            case ACQ_ON_FTN:
                acqOn();
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
                break;
            case ACQ_SAVE_FTN:
            {
                bool saveState = saveAcq(*(tcpData.event_list + i) + 1, tcpData.event_length_list[i]);
                if (saveState)
                {
                    socket_write(ACQ_SAVE_NTF, "", 0);
                }
                else
                {
                    socket_write(ACQ_SAVE_FAILED_NTF, "", 0);
                }

                // 2d일땐 다음 파일 준비
                if (headerParameter.cScanMode == 0)
                {
                    makeSavePath();
                    sendSavePath();
                }
                break;
            }
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
                NVA_TMVInit(0);
                socket_write(NVA_COMPLETE_NTF, "", 0);
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
                threadUsbCopy(*(tcpData.event_list + i) + 1);
                break;
            case ANA_USB_COPY_CANCEL_FTN:
                threadUsbCopyCancel();
                break;
            case ANA_LOAD_FILE_FTN:
                threadSendFileData(*(tcpData.event_list + i) + 1, ANA_LOAD_FILE_NTF);
                break;
            case ANA_LOAD_FILE_CANCEL_FTN:
                threadSendFileCancel();
                break;
            case ANA_LOAD_FILE_WITH_CONFIG_FTN:
                threadSendFileData(*(tcpData.event_list + i) + 1, ANA_LOAD_FILE_WITH_CONFIG_NTF);
                break;
            case ANA_LOAD_CONFIG_FILE_FTN:
                sendLoadConfiFile(*(tcpData.event_list + i) + 1);
                break;
            case ANA_SAVE_CONFIG_FILE_FTN:
                sendSaveConfigFile(*(tcpData.event_list + i) + 1);
                break;
            case ANA_LOAD_TOP_VIEW_FNT:
                threadSendFileData(*(tcpData.event_list + i) + 1, ANA_LOAD_TOP_VIEW_NTF);
                break;
            case WIFI_CHANNEL_LIST_FTN:
            {
                char *out = getWifiInfoList();
                socket_write(WIFI_CHANNEL_LIST_NTF, out, strlen(out));
                free(out);
                break;
            }
            case WIFI_CHANNEL_CHANGE_FTN:
                changeWifiChannel(*(tcpData.event_list + i) + 1);
                server_restart = true;
                socket_write(SOCKET_CLOSE_NTF, "", 0);
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
    acqOff();
    // printf("close :%d \n", tcpData.total_length);
}

int tcpSetKeepAlive(int nSockFd_, int nKeepAlive_, int nKeepAliveIdle_, int nKeepAliveCnt_, int nKeepAliveInterval_)
{
    int nRtn;

    nRtn = setsockopt(nSockFd_, SOL_SOCKET, SO_KEEPALIVE, &nKeepAlive_, sizeof(nKeepAlive_));
    if (nRtn == -1)
    {
        printf("[TCP server]Fail: setsockopt():so_keepalive\n");
        return -1;
    }

    nRtn = setsockopt(nSockFd_, SOL_TCP, TCP_KEEPIDLE, &nKeepAliveIdle_, sizeof(nKeepAliveIdle_));
    if (nRtn == -1)
    {
        printf("[TCP server]Fail: setsockopt():so_keepidle\n");
        return -1;
    }

    nRtn = setsockopt(nSockFd_, SOL_TCP, TCP_KEEPCNT, &nKeepAliveCnt_, sizeof(nKeepAliveCnt_));
    if (nRtn == -1)
    {
        printf("[TCP server]Fail: setsockopt():so_keepcnt\n");
        return -1;
    }

    nRtn = setsockopt(nSockFd_, SOL_TCP, TCP_KEEPINTVL, &nKeepAliveInterval_, sizeof(nKeepAliveInterval_));
    if (nRtn == -1)
    {
        printf("[TCP server]Fail: setsockopt():so_keepintvl\n");
        return -1;
    }
    return nRtn;
}
