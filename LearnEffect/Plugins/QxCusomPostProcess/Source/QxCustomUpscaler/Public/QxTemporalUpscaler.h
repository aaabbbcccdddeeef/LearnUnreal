// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PostProcess/TemporalAA.h"

/**
 * 
 */
class QXCUSTOMUPSCALER_API FQxTemporalUpscaler final : public ITemporalUpscaler
{
public:
    virtual const TCHAR* GetDebugName() const override
    {
        return TEXT("QxTemporalUpscaler");
    }
    virtual void AddPasses(FRDGBuilder& GraphBuilder, const FViewInfo& View, const FPassInputs& PassInputs,
        FRDGTextureRef* OutSceneColorTexture, FIntRect* OutSceneColorViewRect,
        FRDGTextureRef* OutSceneColorHalfResTexture, FIntRect* OutSceneColorHalfResViewRect) const override;

    virtual float GetMinUpsampleResolutionFraction() const override
    {
        return FSceneViewScreenPercentageConfig::kMinTAAUpsampleResolutionFraction;
    }
    virtual float GetMaxUpsampleResolutionFraction() const override
    {
        return FSceneViewScreenPercentageConfig::kMaxTAAUpsampleResolutionFraction;
    }
};
