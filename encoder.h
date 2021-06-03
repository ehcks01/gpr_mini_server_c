#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <pthread.h>

#include "encoder_queue.h"

#define PIN1 3 //BCM 22
#define PIN2 4 //BMC 23
#define SPI_CHANNEL 0
#define SPI_SPEED 12000000
pthread_t threadID;

void GPR_Capture_raw(int deviceNumber);

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

int wiringPi_ready()
{
    if (wiringPiSetup() == -1)
    {
        printf("wiringPiSetup failed\n");
        return 0;
    }
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        printf("wiringPiSPISetup failed\n");
        return 0;
    }

    if (wiringPiISR(PIN1, INT_EDGE_BOTH, &encoder_interrupt) < 0)
    {
        printf("wiringPiISR failed\n");
        return 0;
    }

    pinMode(PIN1, INPUT);
    pinMode(PIN2, INPUT);

    pthread_create(&threadID, NULL, queue_consume, NULL);
    printf("wiringPi setup completed..\n");
    return 1;
}
