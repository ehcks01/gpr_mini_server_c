#ifndef encoder__h
#define encoder__h

#include <stdbool.h>

//i2c 설정 배터리
#define PINBASE  120
#define ADS_ADDR 0x48
#define MIN_BATTERY 11100 //5.5v
#define MAX_BATTERY 24500 //11.7v

//spi 설정 novelda
#define SPI_CHANNEL 0
#define SPI_SPEED 8000000

//gpio 설정 encoder
#define PIN1 21 //BCM 5
#define PIN2 22 //BMC 6
#define ENCODER_POWER_PIN 26 //BCM 12

//gpio 설정 laser
#define LASER_PIN 25 //BCM 26

void encoder_interrupt(void);
bool wiringPi_ready();
int getBatteryPercent();

#endif