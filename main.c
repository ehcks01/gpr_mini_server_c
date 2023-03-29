#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <string.h>
#include "common/usb_control.h"
#include "common/dir_control.h"
#include "common/log.h"
#include "encoder/encoder.h"
#include "NVA/NVA_file.h"
#include "gpr_socket/gpr_socket_data.h"
#include "gpr_socket/gpr_socket.h"
#include "gpr_socket/gpr_socket_protocol.h"
#include "gpr_socket/wifi_selector.h"

int main(char *argc, char *argv[])
{
    /* wifi 채널 변경 시 해당 프로그램이 hostapd 파일을 잘못쓰게 되는 경우, 
      이를 복구 시키려고 했으나, 라즈베리가 부팅 시 여기서 네트워크가 영구 다운되는 증상 때문에 보류 */
    // if (checkHostFile() == false)
    // {
    //     printf("hostFile initialization failed\n");
    // }

    /* initRealPath: 프로그램이 실행되는 경로
       initUsbMountPath: 마운트 되는 usb와 연결되는 폴더경로 지정
       initNVAPath: 노벨다칩 세팅 정보를 저장하는 경로 지정 */
    if ((initRealPath(argv[0]) && initUsbMountPath() && initNVAPath()) == false)
    {
        printf("Path initialization failed");
    };

    //로그정보를 콘솔 창에 print하지 않음
    //log_set_quiet(true);

    //로그정보를 txt에 쓰도록 FILE 지정
    //FILE *fp_log = info_log_add_fp();

    //노벨다칩 정보 세팅
    loadNVASetting();

    //wiringpi 관련 세팅
    if (wiringPi_ready() == false)
    {
        printf("wiringPi initialization failed\n");
    }
    
    while (1)
    {
        //소켓 서버 활성화
        if (socket_ready() == false)
        {
            printf("Socket initialization failed\n");
            break;
        }

        //부팅이 완료된 것을 아두이노로 보냄
        switchServerON();

        //wifi 채널 변경을 하면 서버를 재시작하기 위한 변수
        server_restart = false;

        while (1)
        {
            char buff_rcv[1024];
            int buff_size;

            //클라이언트 소켓 연결을 기다림
            if (!socket_client_accept())
            {
                continue;
            }
            //log_info("%s", "socket_client_accept");

            //클라이언트가 연결된 것을 아두이노로 보냄
            switchClientON();

            //클라이언트가 연결된 것을 앱으로 보냄
            socket_write(CONNECTION_NTF, "", 0);

            //클라이언트에서 보내는 버퍼 받기
            while ((buff_size = socket_receive(buff_rcv)) > 0)
            {
                //버퍼가 온게 이벤트로 잘라지면 처리
                convertEvent(buff_rcv, buff_size);
            }

            //연결이 끊기면 클라이언트 접속종료 처리
            socket_close(); 
            socket_client_done();
            switchClientOFF();

            //log_info("%s", "socket_client_done");

            //wifi 채널을 변경한거라면 소켓 서버 재시작
            if (server_restart)
            {
                break;
            }
        }

        //log_info("%s", "server_restart");
        
        //실제로 여기까지는 오는 경우는 wifi 채널 변경할 때 밖에 없음. 
        switchServerOFF();
        socket_server_done();
        //wifi채널 변경을 위해 라즈베리파이에 명령어 입력
        wifiRestartCommand();
    }
    //fclose(fp_log);
    return 0;
}