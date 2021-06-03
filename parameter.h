struct HeaderParameter
{
    //------파일 헤더 부분 ---------------//
    char strModel[20];          // 모델명
    char strVersion[20];        //버전 정보
    char strDate[40];           //취득날짜정보
    char cResolution;           //해상도 (0:x1 1:x2 2:x4) 하드웨어에도 명령 내려가야함
    unsigned short sLength;     //취득 길이
    char cScanMode;             //스캔모드 0:2D, 1:3D
    char cDepth;                //깊이정보 0:20cm 1:30cm 2:40cm 3:50cm
    char cUnit;                 // 0:Metric(cm) 1:English(inch)
    float fDielectric;          // 유전률
    char strSiteName[20];       // 사이트이름
    char strOperator[20];       // 오퍼레이터
    unsigned short sIterations; // Iteration 하드웨어 인자값
    unsigned short sDacStep;    // DAC STEP 하드웨어 인자값
    unsigned short sDacMax;     // DAC MAX 하드웨어 인자값
    unsigned short sDacMin;     // DAC MIN 하드웨어 인자값
    char cSamplingRate;         // samplingrate 0:26ps 하드웨어 인자값
    char cGainHW;               // Gain 하드웨어 인자값 필요 없어질듯
    char cCoordinate;           // 0:2D path1:X  2:Y
    char cBlowNo;               // BlowNo
    char cSaveMode;             // 0:manual 1:auto
    char cCalibration;          // 0:undone 1:done
    unsigned short sLineCount;
    char cGainSW;
    char cExpGain;
    unsigned short sHPFilter;
    float fNanoTime;
    float fLineNoiseFilter;
    unsigned short sLPFilter;
    char cColorType; //0 : gray, 1 : color1, 2: color2
    //--하드웨어 설정--//
    char cCoarseTune;
    char cMediumTune;
    char cFineTune;
    //------파일 헤더 끝 ---------------//
};

struct HeaderParameter headerParameter = {
    .strModel = "MWGPR_MINI", .strVersion = "3.0.3", .strDate = "", .cResolution = 0, .sLength = 0, .cScanMode = 0, .cDepth = 0, .cUnit = 0, .fDielectric = 5.5, .strSiteName = "", .strOperator = "", .sIterations = 1, .sDacStep = 1, .sDacMax = 1, .sDacMin = 1, .cSamplingRate = 1, .cGainHW = 1, .cCoordinate = 0, .cBlowNo = 1, .cSaveMode = 1, .cCalibration = 0, .sLineCount = 0, .cGainSW = 1, .cExpGain = 1, .sHPFilter = 1, .fNanoTime = 15.960355758666992, .fLineNoiseFilter = 1, .sLPFilter = 1, .cColorType = 0, .cCoarseTune = 8, .cMediumTune = 17, .cFineTune = 1};


const int fixHeaderSize = 256;
const int fixDepthDataSize = 640;