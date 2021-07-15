#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <stdio.h>
#include <unistd.h>

#include "encoder.h"
#include "encoder_queue.h"
#include "../gpr_socket/gpr_socket_acq.h"
#include "../NVA/NVA_CON.h"

bool interruptCheck = false;

void encoder_interrupt(void)
{
    if (interruptCheck)
    {
        return;
    }
    interruptCheck = true;

    if (acqCon.runAcq && (is2DScanMode() || isNotFull3DData()))
    {
        if (digitalRead(PIN1) == digitalRead(PIN2))
        {
            GPR_Capture_raw(0);
            frontRowData();
        }
        else
        {
            backRowData();
        }
    }
    interruptCheck = false;
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
    pinMode(LASER_PIN, OUTPUT);

    printf("wiringPi setup completed..\n");
    return true;
}