#ifndef encoder__h
#define encoder__h

#include <stdbool.h>

//i2c 설정 배터리
#define PINBASE  120
#define ADS_ADDR 0x48
#define MIN_BATTERY 10353
#define MAX_BATTERY 25480

//spi 설정 encoder
#define PIN1 3 //BCM 22
#define PIN2 4 //BMC 23
#define SPI_CHANNEL 0
#define SPI_SPEED 12000000

void encoder_interrupt(void);
bool wiringPi_ready();
int getBatteryPercent();

#endif