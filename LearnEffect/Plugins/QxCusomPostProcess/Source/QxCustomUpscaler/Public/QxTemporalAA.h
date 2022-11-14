#pragma once
#include "CoreMinimal.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/TemporalAA.h"
#include "SceneTextureParameters.h"


namespace QxTemporalAA
{
    FTAAOutputs QxAddTemporalAAPass(
        FRDGBuilder& GraphBuilder,
        const FViewInfo& View,
        const FTAAPassParameters& Inputs,
        const FTemporalAAHistory& InputHistory,
        FTemporalAAHistory* OutputHistory
        );
}
