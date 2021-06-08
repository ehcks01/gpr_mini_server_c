#include <stdio.h>
#include <unistd.h>

#include "encoder/encoder.h"
#include "NVA/NVA_CON.h"
#include "NVA/NVA_file.h"
#include "gpr_socket/gpr_socket.h"
#include "gpr_socket/gpr_socket_protocol.h"

int main()
{
    if (!wiringPi_ready())
    {
        return 0;
    }
    GPR_Init(0);

    usleep(1); //printf문 띄울려고..

    if (!socket_ready())
    {
        return 0;
    }

    loadNVASetting();

    while (1)
    {
        char buff_rcv[1024];
        if (!socket_client_accept())
        {
            continue;
        }

        usleep(1); //printf문 띄울려고..

        socket_write(CONNECTION_NTF, "", 0);
        while (1)
        {
            int buff_size = socket_receive(buff_rcv);
            if (buff_size < 1)
            {
                break;
            }
            socket_read(buff_rcv, buff_size);
        }
        socket_close();
        socket_client_done();
    }
    socket_server_done();

    return 0;
}
