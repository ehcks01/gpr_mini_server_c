#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "common/usb_control.h"
#include "common/dir_control.h"
#include "encoder/encoder.h"
#include "NVA/NVA_file.h"
#include "gpr_socket/gpr_socket.h"
#include "gpr_socket/gpr_socket_protocol.h"

int main(char *argc, char *argv[])
{
    if ((initRealPath(argv[0]) && initUsbMountPath() && initNVAPath()) == false)
    {
        printf("Path initialization failed");
        // return 0;
    };
    loadNVASetting();
    if (wiringPi_ready() == false)
    {
        printf("wiringPi initialization failed\n");
        // return 0;
    }

    while (1)
    {
        if (socket_ready() == false)
        {
            printf("Socket initialization failed\n");
            break;
        }
        server_restart = false;

        while (1)
        {
            char buff_rcv[1024];
            int buff_size;
            usleep(1);
            if (!socket_client_accept())
            {
                continue;
            }
            socket_write(CONNECTION_NTF, "", 0);
            while ((buff_size = socket_receive(buff_rcv)) > 0)
            {
                socket_read(buff_rcv, buff_size);
            }
            socket_close();
            socket_client_done();

            if (server_restart)
            {
                break;
            }
        }
        socket_server_done();
        pclose(popen("sudo service hostapd restart", "r"));
        sleep(2);
    }

    return 0;
}