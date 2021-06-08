#ifndef NVA_SPI__h
#define NVA_SPI__h

void SPI_Transfer(int deviceNumber, unsigned char tx[], int size);
void SetReadBuffer(unsigned char tx[], unsigned char command, int arrayLength, int byteLength);
int SPI_Read(int deviceNumber, unsigned char command, int length);
void SPI_Write(int deviceNumber, unsigned char command, int length, int value);
void SPI_Action(int deviceNumber, unsigned char command);

#endif