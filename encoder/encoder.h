#ifndef encoder__h
#define encoder__h

#include <stdbool.h>

#define PIN1 3 //BCM 22
#define PIN2 4 //BMC 23
#define SPI_CHANNEL 0
#define SPI_SPEED 12000000

void encoder_interrupt(void);
bool wiringPi_ready();

#endif