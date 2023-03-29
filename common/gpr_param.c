#include "gpr_param.h"

//취득 파일 생성에 쓰여지는 헤더파일 정보
struct HeaderParameter headerParameter = {
    .strModel = "MWGPR_MOBILE", .strVersion = "1.0.0", .strDate = "", .cResolution = 0, .sLength = 0, .cScanMode = 0, .cDepth = 0, .cUnit = 0, .fDielectric = 5.5, .strSiteName = "", .strOperator = "", .sIterations = 1, .sDacStep = 1, .sDacMax = 1, .sDacMin = 1, .cSamplingRate = 1, .cGainHW = 1, .cCoordinate = 0, .cBlowNo = 1, .cSaveMode = 1, .cCalibration = 0, .sLineCount = 0, .cGainSW = 1, .cExpGain = 1, .sHPFilter = 1, .fNanoTime = 15.960355758666992, .fLineNoiseFilter = 1, .sLPFilter = 1, .cColorType = 0, .cCoarseTune = 8, .cMediumTune = 17, .cFineTune = 1};

//헤더파일은 256 byte
const int fixHeaderSize = 256;

//펄스 하나당 2byte가 320개인 640byte 공간 차지
const int fixDepthDataSize = 640;

//취득 데이터를 저장하는 루트 폴더는 '프로그램 실행경로/GPR_DATA'
const char *fixDataRootName = "GPR_DATA";

