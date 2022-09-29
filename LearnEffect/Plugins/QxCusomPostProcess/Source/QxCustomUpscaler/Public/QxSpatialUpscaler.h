// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PostProcess/PostProcessUpscale.h"

/**
 * 
 */
class QXCUSTOMUPSCALER_API FQxSpatialUpscaler : public  ISpatialUpscaler
{
public:
    virtual FScreenPassTexture AddPasses(FRDGBuilder& GraphBuilder, const FViewInfo& View, const FInputs& PassInputs) const override;

    const TCHAR* GetDebugName() const override
    {
        return TEXT("FQxSpatialUpscaler");
    }

    virtual ISpatialUpscaler* Fork_GameThread(const FSceneViewFamily& ViewFamily) const override;
};
