#include <unistd.h>
#include <wiringPiSPI.h>

#include "NVA_SPI.h"
#include "../encoder/encoder.h"
#include "../gpr_socket/gpr_socket_acq.h"

void SPI_Transfer(int deviceNumber, unsigned char tx[], int size)
{
    usleep(10);
    wiringPiSPIDataRW(SPI_CHANNEL, tx, size);
}

void SetReadBuffer(unsigned char tx[], unsigned char command, int arrayLength, int byteLength)
{
    tx[0] = command;
    tx[1] = byteLength;

    for (int i = 2; i < arrayLength; i++)
    {
        tx[i] = 0;
    }
}

int SPI_Read(int deviceNumber, unsigned char command, int length)
{
    int reslut = 0;
    int maxSize = 127;
    if (length >= maxSize)
    {
        unsigned char ucPortion = length / maxSize;
        unsigned char ucRest = length % maxSize;

        unsigned char ucData[length];
        for (int i = 0; i < length; i++)
        {
            ucData[i] = 0;
        }

        unsigned int uiData[length / 4];
        for (int i = 0; i < length / 4; i++)
        {
            uiData[i] = 0;
        }

        for (int i = 0; i < ucPortion; i++)
        {
            int byteLength = (i == 0) ? 0x7f : 0xff;
            int arraySize = maxSize + 2;
            unsigned char rawData[arraySize];
            SetReadBuffer(rawData, command, arraySize, byteLength);
            SPI_Transfer(deviceNumber, rawData, arraySize);

            for (int j = 0; j < maxSize; j++)
            {
                ucData[i * maxSize + j] = rawData[j + 2];
            }
        }

        if (ucRest > 0)
        {
            int arraySize = ucRest + 2;
            int byteLength = 0x80 + ucRest;

            unsigned char rawData[arraySize];
            SetReadBuffer(rawData, command, arraySize, byteLength);
            SPI_Transfer(deviceNumber, rawData, arraySize);

            for (int i = 0; i < ucRest; i++)
            {
                ucData[ucPortion * maxSize + i] = rawData[i + 2];
            }
        }

        for (int usTemp = 0; usTemp < length / 4; usTemp++)
        {
            for (int usCounter = 0; usCounter < 4; usCounter++)
            {
                uiData[usTemp] = uiData[usTemp] + ucData[usTemp * 4 + usCounter];
                if (usCounter < 3)
                    uiData[usTemp] = uiData[usTemp] << 8;
            }
        }

        //little endian임
        unsigned char last;
        unsigned char last2;
        for (int i = 0; i < 320; i++)
        {
            acqCon.NVA_readData[i * 2] = uiData[i] & 0xff;
            acqCon.NVA_readData[i * 2 + 1] = (uiData[i] >> 8) & 0xff;

            //byte 11000000 보다 크면
            if (acqCon.NVA_readData[i * 2 + 1] > 0xc0)
            {
                if (i != 0)
                {
                    //0이 아니면 앞 값을 덮어씌움.
                    acqCon.NVA_readData[i * 2] = last;
                    acqCon.NVA_readData[i * 2 + 1] = last2;
                }
                else
                {   
                    //0일 경우는 뒤에 값을 덮어씌움. 연속으로 펄스가 튀는 경우는 고려 안함
                    acqCon.NVA_readData[i * 2] = uiData[i + 1] & 0xff;
                    acqCon.NVA_readData[i * 2 + 1] = (uiData[i + 1] >> 8) & 0xff;
                }
            }
            last = acqCon.NVA_readData[i * 2];
            last2 = acqCon.NVA_readData[i * 2 + 1];
        }
    }
    else
    {
        int arraySize = length + 2;
        unsigned char tx[arraySize];
        SetReadBuffer(tx, command, arraySize, length);

        SPI_Transfer(deviceNumber, tx, arraySize);
        for (int i = 0; i < length; i++)
        {
            reslut += tx[i + 2];
            if (i < length - 1)
            {
                reslut = reslut << 8;
            }
        }
    }

    return reslut;
}

void SPI_Write(int deviceNumber, unsigned char command, int length, int value)
{
    int byteLength = length + 2;
    unsigned char tx[byteLength];
    tx[0] = command + 0x80;
    tx[1] = length;

    do
    {
        tx[--length + 2] = value & 255;
        value = value >> 8;
    } while (length);

    SPI_Transfer(deviceNumber, tx, byteLength);
}

void SPI_Action(int deviceNumber, unsigned char command)
{
    unsigned char tx[2] = {command + 0x80, 0};
    SPI_Transfer(deviceNumber, tx, 2);
}