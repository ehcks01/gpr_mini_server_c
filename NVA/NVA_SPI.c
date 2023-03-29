#include <unistd.h>
#include <wiringPiSPI.h>

#include "NVA_SPI.h"
#include "NVA6100.h"
#include "../encoder/encoder.h"
#include "../gpr_socket/gpr_socket_acq.h"


//노벨다칩과 Wiringpi를 통한 SPI 통신
void SPI_Transfer(int deviceNumber, unsigned char tx[], int size)
{
    wiringPiSPIDataRW(deviceNumber, tx, size);
}

//노벨다칩에 데이터를 받기 위한 버퍼
void SetReadBuffer(unsigned char tx[], unsigned char command, int arrayLength, int byteLength)
{
    tx[0] = command;
    tx[1] = byteLength;

    //데이터를 받는 부분은 0으로 작성
    for (int i = 2; i < arrayLength; i++)
    {
        tx[i] = 0;
    }
}

//노벨다칩에서 데이터를 받음
int SPI_Read(int deviceNumber, unsigned char command, int length)
{
    int reslut = 0;
    int maxSize = 127;
    //127byte 이상이면 펄스 데이터
    if (length >= maxSize)
    {
        unsigned char ucPortion = length / maxSize;
        unsigned char ucRest = length % maxSize;

        //케릭터형 1byte 데이터
        unsigned char ucData[length];
        for (int i = 0; i < length; i++)
        {
            ucData[i] = 0;
        }

        //인트형 4byte 데이터
        unsigned int uiData[length / 4];
        for (int i = 0; i < length / 4; i++)
        {
            uiData[i] = 0;
        }

        //노벨다칩에서 127byte 데이터를 받음
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

        //127byte 외에 나머지 데이터가 있으면 다시 받음
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

        //케릭터형을 인트형 데이터로 변환
        for (int usTemp = 0; usTemp < length / 4; usTemp++)
        {
            for (int usCounter = 0; usCounter < 4; usCounter++)
            {
                uiData[usTemp] = uiData[usTemp] + ucData[usTemp * 4 + usCounter];
                if (usCounter < 3)
                    uiData[usTemp] = uiData[usTemp] << 8;
            }
        }

        //튀는 값 제거(iteration 50에 맞춘거라 다를땐 값 조정 필요.)
        if (uiData[0] > 49000 || uiData[0] < 0)
        {
            //제일 앞에 튀는 값이 있으면 [1]를 [0]에 덮어씌움
            uiData[0] = uiData[1];
        }
        for (int i = 2; i < 319; i++)
        {
            if (uiData[i] > 49000 || uiData[i] < 0)
            {
                //중간에 튀는 값이 있으면 앞뒤 값의 중간 값을 덮어씌움
                uiData[i] = (uiData[i - 1] + uiData[i + 1]) / 2;
            }
        }
        if (uiData[319] > 49000 || uiData[319] < 0)
        {
            //마지막에 튀는 값이 있으면 앞에 값을 덮어씌움
            uiData[319] = uiData[318];
        }

        for (int i = 0; i < 320; i++)
        {
            // uiData[i]를 변경하는 아래 코드의 기능이 명확하지 않아 제외시킴
            // uiData[i] = (unsigned short)(((((uiData)[i] + NVAParam.fOffset) * 100) / NVAParam.fCurrentMaxValue) * 1000);

            //little endian으로 인트형 4byte를 2byte 로 변경
            acqCon.NVA_readData[i * 2] = uiData[i] & 0xff;
            acqCon.NVA_readData[i * 2 + 1] = (uiData[i] >> 8) & 0xff;
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

//노벨다칩에 쓰기 명령어 전송
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

//노벨다칩에 명령어 전송
void SPI_Action(int deviceNumber, unsigned char command)
{
    unsigned char tx[2] = {command + 0x80, 0};
    SPI_Transfer(deviceNumber, tx, 2);
}