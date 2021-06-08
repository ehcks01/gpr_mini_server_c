#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <stdio.h>
#include <unistd.h>

#include "encoder.h"
#include "encoder_queue.h"
#include "../gpr_socket/gpr_socket_acq.h"
#include "../NVA/NVA_CON.h"

void *queue_consume(void *arg)
{
    while (1)
    {
        if (!empty_queue())
        {
            char isFront = delete_queue();
            if (acqCon.runAcq && (is2DScanMode() || isNotFull3DData()))
            {
                if (isFront)
                {
                    GPR_Capture_raw(0);
                    frontRowData();
                }
                else
                {
                    backRowData();
                }
            }
            printf("delete_queue: %d\n", front_queue);
        }
        usleep(1);
    }

    return NULL;
}

void encoder_interrupt(void)
{
    if (acqCon.runAcq)
    {
        if (digitalRead(PIN1) == digitalRead(PIN2))
        {
            printf("inc: %d\n", rear_queue);
            add_queue(1);
        }
        else
        {
            printf("dec: %d\n", rear_queue);
            add_queue(0);
        }
    }
}

bool wiringPi_ready()
{
    if (wiringPiSetup() == -1)
    {
        printf("wiringPiSetup failed\n");
        return false;
    }
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        printf("wiringPiSPISetup failed\n");
        return false;
    }

    if (wiringPiISR(PIN1, INT_EDGE_BOTH, &encoder_interrupt) < 0)
    {
        printf("wiringPiISR failed\n");
        return false;
    }

    pinMode(PIN1, INPUT);
    pinMode(PIN2, INPUT);

    pthread_create(&threadID, NULL, queue_consume, NULL);
    printf("wiringPi setup completed..\n");
    return true;
}