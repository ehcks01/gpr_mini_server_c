#include "gpr_param.h"

struct HeaderParameter headerParameter = {
    .strModel = "MWGPR_MINI", .strVersion = "3.0.3", .strDate = "", .cResolution = 0, .sLength = 0, .cScanMode = 0, .cDepth = 0, .cUnit = 0, .fDielectric = 5.5, .strSiteName = "", .strOperator = "", .sIterations = 1, .sDacStep = 1, .sDacMax = 1, .sDacMin = 1, .cSamplingRate = 1, .cGainHW = 1, .cCoordinate = 0, .cBlowNo = 1, .cSaveMode = 1, .cCalibration = 0, .sLineCount = 0, .cGainSW = 1, .cExpGain = 1, .sHPFilter = 1, .fNanoTime = 15.960355758666992, .fLineNoiseFilter = 1, .sLPFilter = 1, .cColorType = 0, .cCoarseTune = 8, .cMediumTune = 17, .cFineTune = 1};

const int fixHeaderSize = 256;
const int fixDepthDataSize = 640;