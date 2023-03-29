#include <stdio.h>
#include "GPR_TimingMeasurement.h"

#include "NVA_CON.h"
#include "NVA6100.h"
#include "NVA_SPI.h"
#include "../common/gpr_param.h"

//노벨다칩 관련은 gpr_mini 펌웨어를 참고하여 작성됨

const int SampleDelayCoarseTuneMode = 1;
const int SampleDelayMediumTuneMode = 2;
const int RISING_EDGE = 0;
const int FALLING_EDGE = 1;
const int LOW_LEVEL = 0;
const int HIGH_LEVEL = 1;

int TimingFlag = 0;
int SampleDelayCoarseTune = 351;
int SampleDelayMediumTune = 63;
int SampleDelayFineTune = 63;
int FineInMediumAverage = 28;   // One MediumTune element is 31pS, and one FineTune element is 1.7pS.
int MediumInCoarseAverage = 30; // One CoarseTune element is 1nS and one MediumTune element is 31pS

int GPRTimingMin = 0;
int GPRTimingMax = 999;
int GPRTimingTMSample = 10;

struct DelayElement DelayElementCoarse = {.Flag = 2177, .Value = 1};
struct DelayElement DelayElementMedium = {.Flag = 2177, .Value = 1};
struct DelayElement DelayElementFine = {.Flag = 2177, .Value = 1};
struct DelayElement SampleResult = {.Flag = 3203, .Value = 1};

void MeasureWait()
{
    int read_value, wait;

    read_value = SPI_Read(0, SAMPLE_DELAY_COARSE_TUNE, 2);
    read_value = (read_value & 0x03) >> 1;
    read_value = 100000000 / (0x01 << read_value);

    if (read_value > 25000000)
        ;
    else if (read_value > 10000000)
        for (wait = 0; wait < 10; wait++)
            ;
    else if (read_value > 5000000)
        for (wait = 0; wait < 25; wait++)
            ;
    else if (read_value > 1000000)
        for (wait = 0; wait < 50; wait++)
            ;
    else if (read_value > 500000)
        for (wait = 0; wait < 100; wait++)
            ;
    else
        for (wait = 0; wait < 1000; wait++)
            ;
}

void VarSetIntValue(struct DelayElement delayElement, int value)
{
    int old = delayElement.Value;

    if (delayElement.Flag & VAR_IS_INT && GPRTimingMin <= value && value <= GPRTimingMax)
    {
        delayElement.Flag &= ~VAR_IS_AUTO;
        delayElement.Value = value;
    }

    if (old != delayElement.Value)
    {
        if (delayElement.Flag & VAR_IS_VISIBLE)
        {
            int command;
            if (TimingFlag == 1)
                command = SAMPLE_DELAY_COARSE_TUNE;
            else if (TimingFlag == 2)
                command = SAMPLE_DELAY_MEDIUM_TUNE;

            SPI_Write(0, command, 2, delayElement.Value);
        }
    }
}

int GetValue(struct DelayElement delayElement)
{
    int value;

    if (delayElement.Flag & VAR_IS_LIVEREAD)
    {
        value = SUCCESS;
    }
    else if (delayElement.Flag & VAR_IS_REGISTER_READ)
    {
        value = SPI_Read(0, TIMING_CALIBRATION_RESULT, 1);
    }
    else
    {
        value = delayElement.Value;
    }

    return value;
}

int GPR_FindLevel(struct DelayElement delayElement, int delayElementMax, int level, int start)
{
    int delayNo;
    int iterations = 1; // Start with a fast search
    int fastSearch = 1;
    float threshold = 0.9;

    for (int counter = start; counter <= delayElementMax; counter++)
    {
        // Temporary finished
        VarSetIntValue(delayElement, counter);
        SPI_Action(0, TIMING_CALIBRATION_RESET);

        for (int i = 0; i < iterations; i++)
        {
            SPI_Action(0, TIMING_CALIBRATION_SAMPLE);
            MeasureWait(); // it isn't needed if MCLK > 50000000
        }

        int result = GetValue(SampleResult);

        // Check level
        if (
            (level == HIGH_LEVEL && result >= iterations * threshold) ||
            (level == LOW_LEVEL && result <= iterations * (1.0 - threshold)))
        {
            if (fastSearch == 1)
            {
                // Do a thorough search for the edge
                iterations = GPRTimingTMSample;
                counter--;
                fastSearch = 0;
            }
            else
            {
                delayNo = counter;
                VarSetIntValue(delayElement, delayNo);
                break;
            }
        }
    }

    return delayNo;
}

int GPR_FindEdge(struct DelayElement delayElement, int delayElementMax, int edgeDirection, int start)
{
    int counter, delayNo;
    int edgeFound = 0;
    int iterations = 1;
    int fastSearch = 1;
    float threshold = 0.9;

    for (counter = start; counter <= delayElementMax; counter++)
    {
        // Temporary finished
        VarSetIntValue(delayElement, counter);
        SPI_Action(0, TIMING_CALIBRATION_RESET);

        for (int i = 0; i < iterations; i++)
        {
            SPI_Action(0, TIMING_CALIBRATION_SAMPLE);
            MeasureWait(); // it isn't needed if MCLK > 50000000
        }

        int result = GetValue(SampleResult);

        if (edgeDirection == RISING_EDGE && result >= iterations * threshold)
        {
            if (fastSearch == 1)
            {
                iterations = GPRTimingTMSample;
                counter--;
                fastSearch = 0;
            }
            else
            {
                edgeFound = 1;
                if (counter > 0)
                    counter--;
                else
                    counter = 0;

                delayNo = counter;
                VarSetIntValue(delayElement, delayNo);
                break;
            }
        }
        else if (edgeDirection == FALLING_EDGE && result <= iterations * (1.0 - threshold))
        {
            if (fastSearch == 1)
            {
                iterations = GPRTimingTMSample;
                counter--;
                fastSearch = 0;
            }
            else
            {
                edgeFound = 1;
                if (counter > 0)
                    counter--;
                else
                    counter = 0;

                delayNo = counter;
                VarSetIntValue(delayElement, delayNo);
                break;
            }
        }
    }

    if (edgeFound == 0)
        delayNo = counter;
    return delayNo;
}

float GPR_TimingMeasurementValueLiveRead(int deviceNumber)
{
    int start = 3;
    int edge_at;
    float timingValue = 0;

    NVA_VarInit(deviceNumber);
    GPR_Capture_raw(deviceNumber);

    // Find a high level first
    TimingFlag = SampleDelayCoarseTuneMode;
    start = GPR_FindLevel(DelayElementCoarse, SampleDelayCoarseTune, HIGH_LEVEL, start);
    start++;

    edge_at = GPR_FindEdge(DelayElementCoarse, SampleDelayCoarseTune, FALLING_EDGE, start);
    timingValue = edge_at;

    TimingFlag = SampleDelayMediumTuneMode;
    edge_at = GPR_FindEdge(DelayElementMedium, SampleDelayMediumTune, FALLING_EDGE, 0);
    timingValue += edge_at / MediumInCoarseAverage;

    edge_at = GPR_FindEdge(DelayElementFine, SampleDelayFineTune, FALLING_EDGE, 0);
    timingValue += edge_at / FineInMediumAverage / MediumInCoarseAverage;
    TimingFlag = 0;

    return timingValue;
}