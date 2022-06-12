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
    if (checkHostFile() == false)
    {
        printf("hostFile initialization failed\n");
    }

    if ((initRealPath(argv[0]) && initUsbMountPath() && initNVAPath()) == false)
    {
        printf("Path initialization failed");
    };
    //log기록
    log_set_quiet(true);
    FILE *fp_log = info_log_add_fp();
    loadNVASetting();

    if (wiringPi_ready() == false)
    {
        printf("wiringPi initialization failed\n");
    }
    
    while (1)
    {
        if (socket_ready() == false)
        {
            printf("Socket initialization failed\n");
            break;
        }
        switchServerON();
        server_restart = false;

        while (1)
        {
            char buff_rcv[1024];
            int buff_size;
            if (!socket_client_accept())
            {
                continue;
            }
            log_info("%s", "socket_client_accept");
            //tcp data 부분 초기화
            tcpData.event_length = 0;
            tcpData.total_length = 0;
            tcpData.event_list_cnt = 0;

            switchClientON();
            socket_write(CONNECTION_NTF, "", 0);
            while ((buff_size = socket_receive(buff_rcv)) > 0)
            {
                socket_read(buff_rcv, buff_size);
            }
            socket_close();
            socket_client_done();
            switchClientOFF();
            log_info("%s", "socket_client_done");
            if (server_restart)
            {
                break;
            }
        }
        log_info("%s", "server_restart");
        switchServerOFF();
        socket_server_done();
        wifiRestartCommand();
    }
    fclose(fp_log);
    return 0;
}