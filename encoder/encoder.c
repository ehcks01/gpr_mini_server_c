#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <wiringPiI2C.h>
#include <ads1115.h>
#include <stdio.h>
#include <unistd.h>

#include "encoder.h"
#include "encoder_queue.h"
#include "../gpr_socket/gpr_socket_acq.h"
#include "../NVA/NVA_CON.h"

void encoder_interrupt(void)
{
    if (acqCon.runAcq && (is2DScanMode() || isNotFull3DData()))
    {
        int pin2_value = digitalRead(PIN2);
        if (pin2_value == 1)
        {
            int pin1_value = digitalRead(PIN1);
            if (pin2_value == pin1_value)
            {
                GPR_Capture_raw(0);
                frontRowData();
            }
            else
            {
                backRowData();
            }
        }
    }
    usleep(1);
}

bool wiringPi_ready()
{
    if (wiringPiSetup() == -1)
    {
        printf("wiringPiSetup failed\n");
        return false;
    }

    //배터리 정보를 읽기 위한 i2c 설정.
    if (ads1115Setup(PINBASE, ADS_ADDR) == -1)
    {
        printf("ads1115Setup failed\n");
        return false;
    }

    //novelda 정보를 읽기 위한 spi 설정.
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        printf("wiringPiSPISetup failed\n");
        return false;
    }

    //endcoder 정보를 읽기 위한 gpio 설정.
    if (wiringPiISR(PIN2, INT_EDGE_BOTH, &encoder_interrupt) < 0)
    {
        printf("wiringPiISR failed\n");
        return false;
    }

    pinMode(PIN1, INPUT);
    pinMode(PIN2, INPUT);
    pinMode(LASER_PIN, OUTPUT);
    pinMode(ENCODER_POWER_PIN, OUTPUT);

    printf("wiringPi setup completed..\n");
    return true;
}

int getBatteryPercent()
{
    int analog1 = analogRead(PINBASE + 1);
    int fullGauge = MAX_BATTERY - MIN_BATTERY;
    int currentGauge = analog1 - MIN_BATTERY;
    int currentPercent = 0;
    if (currentGauge > 0)
    {
        currentPercent = currentGauge * 100 / fullGauge;
    }
    return currentPercent;
}