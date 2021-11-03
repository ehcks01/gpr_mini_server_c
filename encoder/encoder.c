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
            if ((pin2_value == pin1_value && acqCon.bForwardScan) || (pin2_value != pin1_value && !acqCon.bForwardScan))
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
    //배터리 전압값 읽기
    int analog1 = analogRead(PINBASE + 1);
    //배터리 총 용량
    int fullGauge = MAX_BATTERY - MIN_BATTERY;
    //읽은 전압값의 배터리 용량
    int currentGauge = analog1 - MIN_BATTERY;
    //배터리가 빨리 닳는 구간의 총용량
    int fastConsuomeGauage = (int)((fullGauge * LOW_RATE_BATTERY) / 100);
    //배터리가 빨리 닳는 구간의 총비율(2배 빨리 닳는다고 보고 원래의 비중에서 2배 줄임)
    int fastConsuomRate = (int)(LOW_RATE_BATTERY / 2);
    //읽은 전압값의 일반적인 소모 구간에서의 용량
    int normalGauage = currentGauge - fastConsuomeGauage;

    //30%아래는 비중을 절반으로 떨어드려 15%만 차지하게 할려고함.. 수학적으로 간단하게 가능하면 정리되면 좋을듯 ㅠ
    int currentPercent = 0;
    if (normalGauage > 0)
    {
        int normalConsuomeRate = 100 - fastConsuomRate;
        int normalConsuomeGauage = fullGauge - fastConsuomeGauage;
        currentPercent = (int)(normalGauage * normalConsuomeRate / normalConsuomeGauage + fastConsuomRate);
    }
    else if (currentGauge > 0)
    {
        currentPercent = currentGauge * fastConsuomRate / fastConsuomeGauage;
    }
    return currentPercent;
}