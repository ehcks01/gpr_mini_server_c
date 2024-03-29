#ifndef encoder__h
#define encoder__h

#include <stdbool.h>

//i2c 설정 배터리
#define PINBASE  120
#define ADS_ADDR 0x48
// #define MIN_BATTERY 11100 //5.5v
// #define MAX_BATTERY 24500 //11.7v
#define MAX_BATTERY 24300 //blue: 24700 //black: 23300 //red: 24300 //26300 //약 12.5~6v
#define MIN_BATTERY 11400 //5.5v
#define LOW_RATE_BATTERY 30 //30%부터는 용량을 절반으로 계산

//spi 설정 novelda
#define SPI_CHANNEL 0
#define SPI_SPEED 8000000

//gpio 설정 encoder
#define ENCODER_PIN1 11 
#define ENCODER_PIN2 31 
#define ENCODER_POWER_PIN 30 

//gpio 설정 laser
#define LASER_PIN 25 //BCM 26

extern int battery_gauge;

void encoder_interrupt(void);
bool wiringPi_ready();
void readBatterGauge();
void threadBattery();
void *setBatteryGauge(void *arg);

void switchServerON();
void switchServerOFF();
void switchClientON();
void switchClientOFF();

#endif