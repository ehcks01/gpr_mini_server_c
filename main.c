#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "removeTreeDir.h"
#include "cJSON.h"
#include "gpr_socket_protocol.h"
#include "gpr_socket.h"
#include "encoder.h"
#include "NVA_CON.h"

int main()
{
    if (!wiringPi_ready())
    {
        return 0;
    }
    usleep(1); //printf문 띄울려고..
    GPR_Init(0);
    
    if (!socket_ready())
    {
        return 0;
    }

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

//gcc -o main main.c
//gcc main.c cJSON.c -o main -l wiringPi -pthread