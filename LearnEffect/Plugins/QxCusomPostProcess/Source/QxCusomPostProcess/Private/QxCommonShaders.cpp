#include "QxCommonShaders.h"

IMPLEMENT_GLOBAL_SHADER(FQxNormalDownSamplePS, "/QxPPShaders/QxNormalDownSample.usf", "MainPS", SF_Pixel);

IMPLEMENT_GLOBAL_SHADER(FQxRescaleShader, "/QxPPShaders/QxRescale.usf", "RescalePS", SF_Pixel);


IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FQxTestGlobal, "QxTestGlobal");