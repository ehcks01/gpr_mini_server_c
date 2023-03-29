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
#include "../common/log.h"

int server_socket, client_socket, select_channel = 0;
bool server_restart = NULL;

//소켓 서버 시작. 인터넷 예제 참고
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

//클라이언트 연결 수락. 인터넷 예제 참고
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

    //비정상적인 연결 종료시, 섹션을 완전히 종료하기 위함
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

//클라이언트에서 오는 버퍼 받기
int socket_receive(char *buff_rcv)
{
    return read(client_socket, buff_rcv, sizeof(buff_rcv));
}

//클라이언트 연결 종료
void socket_client_done()
{
    printf("connection close\n");
    close(client_socket);
}

//소켓 서버 종료
void socket_server_done()
{
    close(server_socket);
}

//클라이언트에 버퍼 보내기
void socket_write(char code, char *bytes, int size)
{
    int sendLen = size + 1;
    char *send_buffer = malloc(sendLen + 4);
    int sum = 0;

    //checkSum을 만들기
    send_buffer[0] = 0x7E;  //시작 바이트는 0x7E임
    send_buffer[1] = (sendLen >> 8) & 0xFF; //보내는 버퍼 길이
    send_buffer[2] = sendLen & 0xFF; //보내는 버퍼 길이
    send_buffer[3] = code; //버퍼
    memcpy(send_buffer + 4, bytes, size);

    sum += code;
    for (int i = 0; i < size; i++)
    {
        sum += bytes[i];
    }

    send_buffer[sendLen + 3] = 0xFF - (sum & 0xFF); //마지막은 바이트는 합산한 결과

    write(client_socket, send_buffer, sendLen + 4); //버퍼 보내기
    free(send_buffer);
}

//취득에서 데이터가 라즈베리에 저장되는 경로를 앱에 보냄
void sendSavePath()
{
    char *str = jsonForsendSavePath();
    socket_write(ACQ_SAVE_PATH_NTF, str, strlen(str));
    free(str);
}

//앱에서 보낸 버퍼 처리
void socket_read(char buffer[], int buff_size)
{
    //처음 1byte는 gpr_socket_protocol.h 에 정의된 바이트
    switch (buffer[0])
    {
    case SERVER_INFO_FNT:
    {
        //서버 정보를 앱에 전송. 지금은 배터리 정보만 보냄.
        //log_info("%s", "SERVER_INFO_FNT");
        socket_write(SERVER_INFO_NTF, (char *)&battery_gauge, 1);
        break;
    }
    case HEADER_INFO_FTN:
        //앱에서 설정한 파일 헤더정보를 받음
        //log_info("%s", "HEADER_INFO_FTN");
        setHeaderFromJson(buffer + 1);
        break;
    case ACQ_ON_FTN:
        //앱에서 취득 화면에 진입
        //log_info("%s", "ACQ_ON_FTN");
        acqOn();
        break;
    case ACQ_INFO_FTN:
        //앱에서 설정한 취득 관련 설정을 받음
        //log_info("%s", "ACQ_INFO_FTN");
        setAcqInfoFromJson(buffer + 1);

        if (strlen(acqCon.savePath) == 0)
        {
            makeSavePath();
        }
        sendSavePath();
        break;
    case ACQ_START_FTN:
        //앱에서 취득을 시작
        //log_info("%s", "ACQ_START_FTN");
        startAcq();
        socket_write(ACQ_START_NTF, "", 0);
        break;
    case ACQ_STOP_FTN:
        //앱에서 취득을 종료
        //log_info("%s", "ACQ_STOP_FTN");
        stopAcq();
        socket_write(ACQ_STOP_NTF, "", 0);
        break;
    case ACQ_NON_SAVE_FTN:
        //앱에서 파일 저장을 취소
        //log_info("%s", "ACQ_NON_SAVE_FTN");
        deleteAcqFile();
        break;
    case ACQ_SAVE_FTN:
    {
        //앱에서 파일 저장을 요청
        //log_info("%s", "ACQ_SAVE_FTN");
        bool saveState = saveAcq(buffer + 1, buff_size - 1);
        if (saveState)
        {
            socket_write(ACQ_SAVE_NTF, "", 0);
        }
        else
        {
            socket_write(ACQ_SAVE_FAILED_NTF, "", 0);
        }

        // 2d일땐 다음 파일 저장 경로를 앱에 전송
        if (headerParameter.cScanMode == 0)
        {
            //파일 저장경로 생성
            makeSavePath();
            //파일 저장경로 앱에 전송
            sendSavePath();
        }
        break;
    }
    case ACQ_REFRESH_FTN:
        //앱에서 취득한 데이터를 초기화함
        //log_info("%s", "ACQ_REFRESH_FTN");
        endFileWrite();
        deleteAcqFile();

        socket_write(ACQ_REFRESH_NTF, "", 0);

        if (acqCon.runAcq)
        {
            startAcq();
        }
        break;
    case ACQ_DATE_TIME_FTN:
        //앱에서 취득화면에서 현재시각을 서버에서 전송
        //log_info("%s", "ACQ_DATE_TIME_FTN");
        acqDateTime(buffer + 1, buff_size - 1);
        break;
    case ACQ_ABNORMAL_QUIT:
        //앱에서 취득화면에서 비정상적으로 종료됨 
        //log_info("%s", "ACQ_ABNORMAL_QUIT");
        stopAcq();
        if (is2DScanMode())
        {
            deleteAcqFile(); // 파일 삭제
        }
        else
        {
            deleteAcq3DFolder();
        }
        break;
    case NVA_REQUEST_FTN:
    {
        //앱에서 노벨다칩 설정정보를 서버에 요청
        //log_info("%s", "NVA_REQUEST_FTN");
        char *json = getNVAJson();
        socket_write(NVA_RESPONSE_NTF, json, strlen(json));
        free(json);
        break;
    }
    case NVA_MODIFY_FTN:
        //앱에서 설정한 노벨다칩 설정값을 서버에서 받아 적용
        //log_info("%s", "NVA_MODIFY_FTN");
        setNVASetting(buffer + 1);
        saveNVASetting();
        NVA_TMVInit(0);
        socket_write(NVA_COMPLETE_NTF, "", 0);
        break;
    case ANA_ROOT_DIR_FTN:
        //라즈베리에서 취득 파일이 저장된 최상위 경로를 앱에 보냄
        //log_info("%s", "ANA_ROOT_DIR_FTN");
        sendRootDir();
        break;
    case ANA_DISK_SIZE_FTN:
        //라즈베리의 디스크 사이즈를 앱에 전송
        //log_info("%s", "ANA_DISK_SIZE_FTN");
        sendDiskSize();
        break;
    case ANA_READ_DIR_FTN:
        //해당 경로의 파일,폴더 정보를 앱에 전송
        //log_info("%s", "ANA_READ_DIR_FTN");
        sendReadDir(buffer + 1, ANA_READ_DIR_NTF, false);
        break;
    case ANA_CHECK_DIR_FTN:
        //앱에서 선택한 경로(하위 경로포함)의 파일,폴더 정보를 앱에 전송
        //log_info("%s", "ANA_CHECK_DIR_FTN");
        sendReadDir(buffer + 1, ANA_CHECK_DIR_NTF, true);
        break;
    case ANA_UNCHECK_DIR_FTN:
        //앱에서 선택 해제한 경로(하위 경로포함)의 파일,폴더 정보를 앱에 전송
        //log_info("%s", "ANA_UNCHECK_DIR_FTN");
        sendReadDir(buffer + 1, ANA_UNCHECK_DIR_NTF, true);
        break;
    case ANA_DELETE_FILE_FTN:
        //앱에서 삭제요청한 파일
        //log_info("%s", "ANA_DELETE_FILE_FTN");
        sendDeleteFile(buffer + 1);
        break;
    case ANA_DELETE_FOLDER_FTN:
        //앱에서 삭제요청한 폴더
        //log_info("%s", "ANA_DELETE_FOLDER_FTN");
        sendDeleteFolder(buffer + 1);
        break;
    case ANA_USB_INFO_FTN:
        //앱에서 라즈베리에 마운트된 usb 정보를 요청
        //log_info("%s", "ANA_USB_INFO_FTN");
        sendUsbInFo();
        break;
    case ANA_USB_COPY_FTN:
        //앱에서 라즈베리에서 선택한 파일을 usb로 복사 요청
        //log_info("%s", "ANA_USB_COPY_FTN");
        socket_write(ANA_USB_COPY_NTF, "", 0);
        threadUsbCopy(buffer + 1);
        break;
    case ANA_USB_COPY_CANCEL_FTN:
        //앱에서 usb 복사를 취소
        //log_info("%s", "ANA_USB_COPY_CANCEL_FTN");
        threadUsbCopyCancel();
        break;
    case ANA_LOAD_FILE_FTN:
        //앱에서 파일 데이터 불러오기 요청
        //log_info("%s", "ANA_LOAD_FILE_FTN");
        threadSendFileData(buffer + 1, ANA_LOAD_FILE_NTF);
        break;
    case ANA_LOAD_FILE_CANCEL_FTN:
        //앱에서 파일 데이터 불러오기 취소 
        //log_info("%s", "ANA_LOAD_FILE_CANCEL_FTN");
        threadSendFileCancel();
        break;
    case ANA_LOAD_FILE_WITH_CONFIG_FTN:
        //앱에서 파일 및 파일 설정 정보를 같이 불러오기 요청
        //log_info("%s", "ANA_LOAD_FILE_WITH_CONFIG_FTN");
        threadSendFileData(buffer + 1, ANA_LOAD_FILE_WITH_CONFIG_NTF);
        break;
    case ANA_LOAD_CONFIG_FILE_FTN:
        //앱에서 파일 설정 정보만 불러오기 요청
        //log_info("%s", "ANA_LOAD_CONFIG_FILE_FTN");
        sendLoadConfiFile(buffer + 1);
        break;
    case ANA_SAVE_CONFIG_FILE_FTN:
        //앱에서 파일 설정 정보를 라즈베리에 저장하기 위해 전송
        //log_info("%s", "ANA_SAVE_CONFIG_FILE_FTN");
        sendSaveConfigFile(buffer + 1);
        break;
    case ANA_LOAD_TOP_VIEW_FNT:
        //앱에서 3D 데이터에서 Topview 보기를 요청
        //log_info("%s", "ANA_LOAD_TOP_VIEW_FNT");
        threadSendFileData(buffer + 1, ANA_LOAD_TOP_VIEW_NTF);
        break;
    case WIFI_CHANNEL_LIST_FTN:
    {
        //앱에서 와이파이 채널 상태정보를 요청
        //log_info("%s", "WIFI_CHANNEL_LIST_FTN");
        char *out = getWifiInfoList();
        socket_write(WIFI_CHANNEL_LIST_NTF, out, strlen(out));
        free(out);
        break;
    }
    case WIFI_CHANNEL_CHANGE_FTN:
    {
        //앱에서 와이파이 채널 변경을 요청
        char *ch = buffer + 1;
        socket_write(SOCKET_CLOSE_NTF, "", 0);
        select_channel = atoi(ch);
        //log_info("%s", "WIFI_CHANNEL_CHANGE_FTN");
        changeWifiChannel(ch);
        server_restart = true;
        break;
    }
    default:
        break;
    }
    //log_info("%d DONE", buffer[0]);
}

//클라이언트 연결 종료
void socket_close()
{
    stopAcq();
    acqOff();
    // printf("close :%d \n", tcpData.total_length);
}


//비정상적인 연결 종료시, 섹션을 완전히 종료하기 위함. 인터넷 예제 참고
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
