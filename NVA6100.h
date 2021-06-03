/***************************************************************

  NVA6100 register constants
  For the detailed contents, refer page 26 of the datasheet, NVA-6100 

  NVA6100 레지스터 변수, 자세한 내용은 데이터시트 26페이지 참조

**************************************************************/

#define FORCE_ZERO 0x00 // R 8bit
#define FORCE_ONE 0x01 // R 8bit
#define CHIP_ID 0x02 // R 16bit
#define SAMPLER_OUTPUT_BUFFER 0x20 // R 0 - 16kbit
#define SAMPLER_READOUTCTRL 0x21 // RW 16bit
#define SAMPLER_CTRL 0x22 // R 8bit
#define THRESHOLDER_POWER_DOWN 0x23 // R 8bit
#define LOAD_OUTPUT_BUFFER 0x24 // W Action
#define RESET_COUNTERS 0x25 // W Action
#define SAMPLER_INPUT_CTRL 0x26 // R 8bit
#define THRESHOLDER_CTRL 0x27 // R 8bit
#define COUNTER_BIT_SELECTOR_OUTPUT 0x28 // R 16bit
#define SAMPLINGRATE_MEASUREMENT_RESULT 0x2c // R 8bit
#define SAMPLINGRATE_MEASUREMENT_SAMPLE 0x2d // W Action
#define SAMPLINGRATE_MEASUREMENT_RESET 0x2e // W Action
#define FOCUS_PULSES_PER_STEP 0x30 // RW 32bit
#define NORMAL_PULSES_PER_STEP 0x31 // RW 32bit
#define DAC_FIRST_ITERATION_SETUP_TIME 0x32 // RW 16bit
#define DAC_FIRST_STEP_SETUP_TIME 0x33 // RW 16bit
#define DAC_REGULAR_SETUP_TIME 0x34 // RW 16bit
#define DAC_LAST_ITERATION_HOLD_TIME 0x35 // RW 16bit
#define DAC_LAST_STEP_HOLD_TIME 0x36 // RW 16bit
#define DAC_REGULAR_HOLD_TIME 0x37 // RW 16bit
#define SWEEP_MAIN_CTRL 0x38 // RW 8bit
#define DAC_MAX 0x39 // RW 16bit
#define DAC_MIN 0x3a // RW 16bit
#define DAC_STEP 0x3b // RW 16bit
#define ITERATIONS 0x3c // RW 16bit
#define FOCUS_MAX 0x3d // RW 16bit
#define FOCUS_MIN 0x3e // RW 16bit
#define FOCUS_SETUP_TIME 0x40 // RW 8bit
#define FOCUS_HOLD_TIME 0x41 // RW 8bit
#define SWEEP_CLK_CTRL 0x42 // RW 8bit
#define START_SWEEP 0x43 // W Action
#define RESET_SWEEP_CONTROLLER 0x44 // W Action
#define SWEEP_CONTROLLER_STATUS 0x47 // R 16bit
#define PGC_CTRL 0x50 // RW 8bit
#define DAC_CTRL 0x58 // RW 16bit
#define MCLK_CTRL 0x60 // RW 8bit
#define STAGGERED_PRF_CTRL 0x61 // RW 8bit
#define STAGGERED_PRF_DELAY 0x62 // RW 8bit
#define STAGGERED_PRF_RESET 0x63 // W Action
#define LFSR5_TAP_ENABLE 0x64 // RW 16bit
#define LFSR4_TAP_ENABLE 0x65 // RW 16bit
#define LFSR3_TAP_ENABLE 0x66 // RW 16bit
#define LFSR2_TAP_ENABLE 0x67 // RW 16bit
#define LFSR1_TAP_ENABLE 0x68 // RW 16bit
#define LFSR0_TAP_ENABLE 0x69 // RW 16bit
#define TIMING_CTRL 0x6a // RW 8bit
#define SAMPLE_DELAY_COARSE_TUNE 0x6b // RW 16bit
#define SAMPLE_DELAY_MEDIUM_TUNE 0x6c // RW 8bit
#define SAMPLE_DELAY_FINE_TUNE 0x6d // RW 8bit
#define SEND_PULSE_DELAY_COARSE_TUNE 0x6e // RW 16bit
#define SEND_PULSE_DELAY_MEDIUM_TUNE 0x6f // RW 8bit
#define SEND_PULSE_DELAY_FINE_TUNE 0x70 // RW 8bit
#define TIMING_CALIBRATION_CTRL 0x71 // RW 8bit
#define TIMING_CALIBRATION_SAMPLE 0x72 // W Action
#define TIMING_CALIBRATION_RESET 0x73 // W Action
#define TIMING_CALIBRATION_RESULT 0x74 // R 8bit
#define MCLK_OUTPUT_CTRL 0x75

int ChipID = 0;
int SamplingCount = 320;
int DownSamping = 0;
int SamplingRate = 0;
int Gain = 6;
int Iterations = 50;
int DACMin = 2457;
int DACMax = 5734;
int DACStep = 8;
int CounterBitSelOut = 31;
int SamplingRateMeasResult = 198;
int SweepControllerStatus = 5733;
int TimingMeasResult = 0;
int FocusMax = 4000;
int FocusMin = 2000;