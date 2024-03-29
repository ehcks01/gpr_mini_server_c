#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <wiringPiI2C.h>
#include <ads1115.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "encoder.h"
#include "encoder_queue.h"
#include "../gpr_socket/gpr_socket_acq.h"
#include "../NVA/NVA_CON.h"

int battery_gauge = 0; //배터리양
int arduino_serial_fd; //아두이나와 연결된 시리얼포트

//엔코더 인터럽트 발생 처리
void encoder_interrupt(void)
{
    //앱이 취득상태인지, 2D 모드인지, 3D 모드면 목표거리를 채웠는지
    if (acqCon.runAcq && (is2DScanMode() || isNotFull3DData()))
    {
        // 1, 0 값을 다 읽으면 분해능이 200이 나오므로 1일때만 읽음 (바퀴 한바퀴에 200번 인터럽트 발생)
        int pin2_value = digitalRead(ENCODER_PIN2);
        if (pin2_value == 1)
        {
            int pin1_value = digitalRead(ENCODER_PIN1);

            //앱에서 설정한 취득방향이 전방인지 후방인지 확인해서 바퀴방향 설정
            if ((pin2_value != pin1_value && acqCon.bForwardScan) || (pin2_value == pin1_value && !acqCon.bForwardScan))
            {
                GPR_Capture_raw(0); //노벨다칩에서 펄스 데이터 획득
                frontRowData(); //취득한 펄스 데이터 기록
            }
            else
            {
                backRowData(); // 뒤로 이동 처리
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

    //led를 컨트롤하는 아두이노와 시리얼 세팅
    if ((arduino_serial_fd = serialOpen("/dev/ttyS0", 57600)) < 0)
    {
        printf("arduino_serial_fd failed\n");
        return false;
    }
    serialPuts(arduino_serial_fd, "boot completed.\n");

    // 배터리 정보를 읽기 위한 i2c 설정.
    if (ads1115Setup(PINBASE, ADS_ADDR) == -1)
    {
        printf("ads1115Setup failed\n");
        return false;
    }

    // novelda 정보를 읽기 위한 spi 설정.
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        printf("wiringPiSPISetup failed\n");
        return false;
    }

    // endcoder 정보를 읽기 위한 gpio 설정.
    if (wiringPiISR(ENCODER_PIN2, INT_EDGE_BOTH, &encoder_interrupt) < 0)
    {
        printf("wiringPiISR failed\n");
        return false;
    }

    //엔코더와 연결핀 설정
    pinMode(ENCODER_PIN1, INPUT);
    pinMode(ENCODER_PIN2, INPUT);
    pinMode(ENCODER_POWER_PIN, OUTPUT);

     //레이저와 연결핀 설정
    pinMode(LASER_PIN, OUTPUT);
    
    //주기적으로 배터리 정보를 앱으로 보내는 쓰레드 설정
    threadBattery();
    printf("wiringPi setup completed..\n");
    return true;
}

//배터리 정보를 주기적으로 아두이노로 보내기 위한 쓰레드
void threadBattery()
{
    pthread_t batteryThread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&batteryThread, &attr, setBatteryGauge, NULL);
}

//배터리 일정시간 마다 읽어서 아두이노로 전송
void *setBatteryGauge(void *arg)
{
    while (true)
    {
        readBatterGauge();
        char str[50];
        serialPuts(arduino_serial_fd, str);
        sleep(5);
    }
}

void readBatterGauge()
{
    // 배터리 전압값 읽기
    int analog1 = analogRead(PINBASE + 1);
    // printf("battery: %d\n", analog1);
    // 배터리 총 용량
    int fullGauge = MAX_BATTERY - MIN_BATTERY;
    // 읽은 전압값의 배터리 용량
    int currentGauge = analog1 - MIN_BATTERY;
    // 배터리가 빨리 닳는 구간의 총용량
    int fastConsuomeGauage = (int)((fullGauge * LOW_RATE_BATTERY) / 100);
    // 배터리가 빨리 닳는 구간의 총비율(2배 빨리 닳는다고 보고 원래의 비중에서 2배 줄임)
    int fastConsuomRate = (int)(LOW_RATE_BATTERY / 2);
    // 읽은 전압값의 일반적인 소모 구간에서의 용량
    int normalGauage = currentGauge - fastConsuomeGauage;

    // 30%아래는 비중을 절반으로 떨어드려 15%만 차지하게 할려고함.. 수학적으로 간단하게 가능하면 정리되면 좋을듯 ㅠ
    if (normalGauage > 0)
    {
        int normalConsumeRate = 100 - fastConsuomRate;
        int normalConsumeGauage = fullGauge - fastConsuomeGauage;
        battery_gauge = (int)(normalGauage * normalConsumeRate / normalConsumeGauage + fastConsuomRate);
    }
    else if (currentGauge > 0)
    {
        battery_gauge = currentGauge * fastConsuomRate / fastConsuomeGauage;
    }
    else
    {
        battery_gauge = 0;
    }
}

//소켓서버 시행 시 아두이노 색상 변경
void switchServerON()
{
    serialPuts(arduino_serial_fd, "server on\n");
}

//소켓서버 죵료 시 아두이노 색상 변경
void switchServerOFF()
{
    serialPuts(arduino_serial_fd, "server off\n");
}

//소켓클라이언트 접속 시 아두이노 색상 변경
void switchClientON()
{
    serialPuts(arduino_serial_fd, "client on\n");
}

//소켓클라이언트 종료 시 아두이노 색상 변경
void switchClientOFF()
{
    serialPuts(arduino_serial_fd, "client off\n");
}
