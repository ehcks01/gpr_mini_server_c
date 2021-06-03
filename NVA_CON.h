#include "NVA6100.h"
#include "NVA_SPI.h"

int m_uCoarseTune = 8;
int m_uMediumTune = 17;
int m_uFineTune = 1;
int m_uCoarseTuneAdjust = 0;
int m_uMediumTuneAdjust = 0;
int m_uFineTuneAdjust = 0;

void NVA_KitConncetCheck(int deviceNumber)
{
    SPI_Read(deviceNumber, FORCE_ZERO, 1);
    ChipID = SPI_Read(deviceNumber, CHIP_ID, 2);
    SPI_Read(deviceNumber, FORCE_ZERO, 1);
    SPI_Action(deviceNumber, RESET_SWEEP_CONTROLLER);

    SPI_Read(deviceNumber, FORCE_ZERO, 1);
    ChipID = SPI_Read(deviceNumber, CHIP_ID, 2);
    CounterBitSelOut = SPI_Read(deviceNumber, COUNTER_BIT_SELECTOR_OUTPUT, 2);
    SamplingRateMeasResult = SPI_Read(deviceNumber, SAMPLINGRATE_MEASUREMENT_RESULT, 1);
    SweepControllerStatus = SPI_Read(deviceNumber, SWEEP_CONTROLLER_STATUS, 2);
    TimingMeasResult = SPI_Read(deviceNumber, TIMING_CALIBRATION_RESULT, 1);

    // Open the kit:::ForceZero and Read Chip ID
    SPI_Read(deviceNumber, FORCE_ZERO, 1);
    SPI_Read(deviceNumber, FORCE_ONE, 1);
    ChipID = SPI_Read(deviceNumber, CHIP_ID, 2);
    CounterBitSelOut = SPI_Read(deviceNumber, COUNTER_BIT_SELECTOR_OUTPUT, 2);
    SamplingRateMeasResult = SPI_Read(deviceNumber, SAMPLINGRATE_MEASUREMENT_RESULT, 1);
    SweepControllerStatus = SPI_Read(deviceNumber, SWEEP_CONTROLLER_STATUS, 2);
    TimingMeasResult = SPI_Read(deviceNumber, TIMING_CALIBRATION_RESULT, 1);

    printf("ChipID: %d\n", ChipID);
}
void NVA_VarInit(int deviceNumber)
{
    // Sampler Readout Control
    // [11:7]: CounterMSB
    // [ 6:2]: CounterLSB
    // [ 1:0]: Downsampling 0: 512, 1: 256, 2: 128, 3: 64
    if (DownSamping == 0)
    {
        SPI_Write(deviceNumber, SAMPLER_READOUTCTRL, 2, 0x0f80);
    }
    else if (DownSamping == 1)
    {
        SPI_Write(deviceNumber, SAMPLER_READOUTCTRL, 2, 0x0f81);
    }
    else if (DownSamping == 2)
    {
        SPI_Write(deviceNumber, SAMPLER_READOUTCTRL, 2, 0x0f82);
    }
    else if (DownSamping == 3)
    {
        SPI_Write(deviceNumber, SAMPLER_READOUTCTRL, 2, 0x0f83);
    }

    // SamplerCtrl: Select the effective sampling interval in the sampler
    // Sampling rate 0x0000: 26pS, 0x0001: 52pS, 0x0002: 280pS, 0x0003: Not used
    if (SamplingRate == 0)
    {
        SPI_Write(deviceNumber, SAMPLER_CTRL, 1, SamplingRate);
    }
    else if (SamplingRate == 1)
    {
        SPI_Write(deviceNumber, SAMPLER_CTRL, 1, SamplingRate);
    }
    else if (SamplingRate == 2)
    {
        SPI_Write(deviceNumber, SAMPLER_CTRL, 1, SamplingRate);
    }
    else if (SamplingRate == 3)
    {
        SPI_Write(deviceNumber, SAMPLER_CTRL, 1, SamplingRate);
    }

    // ThresholderPowerdown: [1]: Manual power down, [0]: Disable Automatic power down
    SPI_Write(deviceNumber, THRESHOLDER_POWER_DOWN, 1, 0x01);

    // SamplerInputCtrl:
    SPI_Write(deviceNumber, SAMPLER_INPUT_CTRL, 1, 0x00);

    // ThresholderCtrl, Selecting gain
    if (Gain == 7)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 4 << 2);
    }
    else if (Gain == 6)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 2 << 2);
    }
    else if (Gain == 5)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 6 << 2);
    }
    else if (Gain == 4)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 1 << 2);
    }
    else if (Gain == 3)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 5 << 2);
    }
    else if (Gain == 2)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 3 << 2);
    }
    else if (Gain == 1)
    {
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 0x00);
        SPI_Write(deviceNumber, THRESHOLDER_CTRL, 1, 7 << 2);
    }

    // FocusPulsesPerStep:
    SPI_Write(deviceNumber, FOCUS_PULSES_PER_STEP, 4, 0x01);

    // NormalPulsesPerStep:
    SPI_Write(deviceNumber, NORMAL_PULSES_PER_STEP, 4, 0x01);

    // DACFirstInterationSetupTime:
    SPI_Write(deviceNumber, DAC_FIRST_ITERATION_SETUP_TIME, 2, 0x00);

    // DACFirstStepSetupTime:
    SPI_Write(deviceNumber, DAC_FIRST_STEP_SETUP_TIME, 2, 0x00);

    // DACRegularSetupTime:
    SPI_Write(deviceNumber, DAC_REGULAR_SETUP_TIME, 2, 0x00);

    // DACLastIterationHoldTime:
    SPI_Write(deviceNumber, DAC_LAST_ITERATION_HOLD_TIME, 2, 0x00);

    // DACLastStepHoldTime
    SPI_Write(deviceNumber, DAC_LAST_STEP_HOLD_TIME, 2, 0x00);

    // DACRegularHoldTime
    SPI_Write(deviceNumber, DAC_REGULAR_HOLD_TIME, 2, 0x00);

    // SweepMainCTRL
    SPI_Write(deviceNumber, SWEEP_MAIN_CTRL, 1, 0x01);

    // DACMax: Default: 5734 -> 0x1666
    SPI_Write(deviceNumber, DAC_MAX, 2, DACMax);

    // DACMin: Default: 2457 -> 0x0999
    SPI_Write(deviceNumber, DAC_MIN, 2, DACMin);

    // DACStep: Step size of DAC sweep
    SPI_Write(deviceNumber, DAC_STEP, 2, DACStep);

    // Iterations: Default: 50 -> 0x0032
    SPI_Write(deviceNumber, ITERATIONS, 2, Iterations);

    // FocusMax: Defines the focus max value, Default: 4000 -> 0x0fa0
    SPI_Write(deviceNumber, FOCUS_MAX, 2, FocusMax); //

    // FocusMin: Defines the focus min value, Default: 2000 -> 0x07d0
    SPI_Write(deviceNumber, FOCUS_MIN, 2, FocusMin); //

    // FocusSetupTime: Setup time, in MCLK periods,
    // for the Focus signal going from the sweep controller to the sampler
    SPI_Write(deviceNumber, FOCUS_SETUP_TIME, 1, 0x00);

    // FocusHoldTime: Hold time, in MCLK periods,
    // for the focus signal going from the sweep controller to the sampler
    SPI_Write(deviceNumber, FOCUS_HOLD_TIME, 1, 0x00); //

    // SweepClkCTRL:
    SPI_Write(deviceNumber, SWEEP_CLK_CTRL, 1, 0x00); //

    // PGCtrl: 573333
    // [7:6] 0: Pulse generator is turned off, 1: Low-band pulse generator, 2: Medium-band pulse generator, 3: Pulse generator is turned off
    // [2:1] 0: Slow, 1: Nominal, 2: Fast
    // Default: Low band pulse generator, Nominal
    SPI_Write(deviceNumber, PGC_CTRL, 1, 0x40);

    // DACCtrl:DAC Source Sweep Controller
    // [15]: 1: DAC is updated by Sweep Controller, 0: DAC is GPR_Configured by the DAC Value manual
    // Default: 1
    SPI_Write(deviceNumber, DAC_CTRL, 2, 0x8000);

    // MCLKCtrl:
    // [4]: 1: Clock enable, 0: Clock disable
    // [3:1]: Clock divider
    // [0]: Divided Clock shaper, 0: Disable, 1: Enable
    // Default: Clock enable, Clock divider: 1
    SPI_Write(deviceNumber, MCLK_CTRL, 1, 0x12);

    // StaggeredPRFCtrl:
    // [2]:StaggeredPRFEnable, 1: Enable, 0: Disable
    // [1]:DelayClkSelect, 1: Undelayed staggered MCLK, 0: SendPulse delay line
    // [0]:DelayClkSampleeWhenReady, 1: Enable, 0: Deisable
    // Default: 1:Enable, 0:SendPulse delay line, 0:Disable
    SPI_Write(deviceNumber, STAGGERED_PRF_CTRL, 1, 0x04);

    // Staggered PRFDelay:
    // [7:0] Delay the sample signal by 0-255 divide clock periods
    // Default: 0
    SPI_Write(deviceNumber, STAGGERED_PRF_DELAY, 1, 0x00);

    // LFSRnT
    SPI_Write(deviceNumber, LFSR5_TAP_ENABLE, 2, 0x4f20);
    SPI_Write(deviceNumber, LFSR4_TAP_ENABLE, 2, 0x4a00);
    SPI_Write(deviceNumber, LFSR3_TAP_ENABLE, 2, 0x4801);
    SPI_Write(deviceNumber, LFSR2_TAP_ENABLE, 2, 0x4120);
    SPI_Write(deviceNumber, LFSR1_TAP_ENABLE, 2, 0x4108);
    SPI_Write(deviceNumber, LFSR0_TAP_ENABLE, 2, 0x4120);

    // TimingCtrl
    SPI_Write(deviceNumber, TIMING_CTRL, 1, 0x00);

    // SampleDelayCoarseTune
    // [8:0] Sample Delay Coarse Tune: Delays the sample signal using 0-351 delay elements, each with a delay of ~1ns.
    // Original comment: frame offset, sampleDelayCoarseTune, 0m(2), 2m(12) 4m(23) 6m(34) 8m(44)
    // RADAR scope default: 0, Matlab source default: 4
    SPI_Write(deviceNumber, SAMPLE_DELAY_COARSE_TUNE, 2, m_uCoarseTune + m_uCoarseTuneAdjust);

    // SampleDelayMediumTune
    // [5:0] Sample Delay Mdeium Tune: Delays the sample signal using 0-63 delay elements, each with a delay of ~31ps.
    // Original comment: sampleDelayMediumTune, 0m(10), 2m(27) 4m(15) 6m(2) 8m(20)
    // RADAR scope default: 0, Matlab source default: 30
    SPI_Write(deviceNumber, SAMPLE_DELAY_MEDIUM_TUNE, 1, m_uMediumTune + m_uMediumTuneAdjust);

    // SampleDelayFineTune
    // [5:0] Sample Delay Fine Tune: Delays the sample signal using 0-63 delay elements, each with a delay of ~1.7ps.
    SPI_Write(deviceNumber, SAMPLE_DELAY_FINE_TUNE, 1, m_uFineTune + m_uFineTuneAdjust);

    // SendPulseDelayCoarseTune
    // [8:0]: Send Pulse Delay Coarse Tune: Delays the send pulse signal using 0-351 delay elements, each with a delay of ~1ns.
    SPI_Write(deviceNumber, SEND_PULSE_DELAY_COARSE_TUNE, 2, 0x00);

    // SendSampleDelayMediumTune
    // [5:0] Sample Delay Mdeium Tune: Delays the pulse signal using 0-63 delay elements, each with a delay of ~31ps.
    SPI_Write(deviceNumber, SEND_PULSE_DELAY_MEDIUM_TUNE, 1, 0x00);

    // SendSampleDelayFineTune
    // [5:0] Sample Delay Fine Tune: Delays the pulse signal using 0-63 delay elements, each with a delay of ~1.7ps.
    SPI_Write(deviceNumber, SEND_PULSE_DELAY_FINE_TUNE, 1, 0x00);

    // Timing CalibrationCtrl
    // [0]: 1: N/A, 0: SendPulse signal
    // Default: 0
    SPI_Write(deviceNumber, TIMING_CALIBRATION_CTRL, 1, 0x00);

    // MClkOutputCtrl
    // [1]: MclkDelayedOutEnable
    // [0]: MclkDelayedOutsource: 0: Sample signal, 1: SendPulse signal
    // Default: 0x0001
    SPI_Write(deviceNumber, MCLK_OUTPUT_CTRL, 8, 0x01);
}

void NVA_Init(int deviceNumber)
{
    NVA_KitConncetCheck(deviceNumber);
    NVA_VarInit(deviceNumber);
}

void GPR_Init(int deviceNumber)
{
    /* Timing Measurement Value Live Read function, 온도보상을 위한 Timing Measurement Value Live(이하 TMV) 함수
	동일 측정조건에서 온도가 올라가면 파형이 지면에 가까워지고, 온도가 내려가면 파형이 지면에서 멀어진다.   
	해당 문제를 보상하기 위해 TMV 함수가 존재하며 해당 값을 통해 온도보상을 할 수 있다. 
	자세한 내용은 결과보고서 "8.1. 온도영향 최소화를 위한 알고리즘" 을 참조한다. */

    float Temp;
    float m_fTMV = 0;
    float m_fReferenceTMV = 15.45;

    // m_uCoarseTuneAdjust = 0;
    // m_uMediumTuneAdjust = 0;
    // m_uFineTuneAdjust = 0;

    // TMV값이 변동이 심하기 때문에 10번의 데이터를 취득한 후 평균을 사용한다.
    // for (int i = 0; i < 10; i++) {
    //   m_fTMV += GPR_TimingMeasurementValueLiveRead(deviceNumber); // TMV 취득 함수 수행, 해당 값의 단위는 nS
    // }

    // m_fTMV /= 10;

    // // 레퍼런스 TMV값과 위 함수에서 취득한 TMV값을 비교한 후 최종 Tune 값 저장
    // // 원래라면 (취득한 TMV - 레퍼런스) 이지만 실제 시험한 결과 ((취득한 TMV-레퍼런스)/2)를 취해줘야 실제값에 가까워서 /2 취함
    // Temp = (m_fTMV - m_fReferenceTMV) / 2;

    // if (Temp >= 1) {
    //   m_uCoarseTuneAdjust = floor(Temp / 1);
    //   Temp = Temp - m_uCoarseTuneAdjust;
    // }

    // if (Temp >= 0.031) {
    //   m_uMediumTuneAdjust = floor(Temp / 0.031);
    //   Temp = Temp - m_uMediumTuneAdjust * 0.031;
    // }

    // if (Temp >= 0.0017) {
    //   m_uFineTuneAdjust = floor(Temp / 0.0017);
    // }

    // // If m_cGNDCut is off, 0cm is not the surface of the object.
    // // It has a room from 0cm. Hence m_uMediumTuneAdjust has -3 value to make a room from 0cm.
    // bool m_cGNDCut = false;
    // if (m_cGNDCut == false) m_uMediumTuneAdjust = m_uMediumTuneAdjust - 3;

    // 초기화
    NVA_Init(deviceNumber);
}

void GPR_Capture_raw(int deviceNumber)
{
    int SweepControllerStatus = 0; // For checking the sweeping, Sweeping 여부 확인용 변수

    SPI_Action(deviceNumber, RESET_COUNTERS); // NVA6100 내부 카운터 초기화명령 수행
    SPI_Action(deviceNumber, START_SWEEP);    // NVA6100 데이터 스윕(취득) 시작명령 수행

    do
    {
        // 스윕 컨트롤러의 상태를 확인하여 SweepContollerStatus에 저장
        SweepControllerStatus = SPI_Read(deviceNumber, SWEEP_CONTROLLER_STATUS, 2);
        // For checking Sweeping[15], Focus[14] and SampleEnable[13] of SweepControllerStatus,
        SweepControllerStatus >>= 13;

        // Sweeping[15], Focus[14] and SampleEnable[13] of SweepControllerStatus are Zero,
        // Sweeping[15], Focus[14] and SampleEnable[13] 가 0이면 while문 종료
        // Initialize SweepControllerStatus, 아니면 계속 수행
    } while (SweepControllerStatus != 0);

    // 데이터를 버퍼에 담기 위해 LOAD_OUTPUT_BUFFER명령 수행
    SPI_Action(deviceNumber, LOAD_OUTPUT_BUFFER);
    // 버퍼에 담긴 데이터를 SPI 통신을 통해 원하는 변수로 저장
    SPI_Read(deviceNumber, SAMPLER_OUTPUT_BUFFER, SamplingCount * 4);
}