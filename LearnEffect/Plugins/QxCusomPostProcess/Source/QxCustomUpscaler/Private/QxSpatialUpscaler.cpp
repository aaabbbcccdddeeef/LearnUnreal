// Fill out your copyright notice in the Description page of Project Settings.


#include "QxSpatialUpscaler.h"



FScreenPassTexture FQxSpatialUpscaler::AddPasses(FRDGBuilder& GraphBuilder,
    const FViewInfo& View,
    const FInputs& PassInputs) const
{
    UE_LOG(LogTemp, Warning, TEXT("Test"));
    FScreenPassTexture FinalOut = PassInputs.SceneColor;
    return MoveTemp(FinalOut);
}

ISpatialUpscaler* FQxSpatialUpscaler::Fork_GameThread(const FSceneViewFamily& ViewFamily) const
{
    // 	// the object we return here will get deleted by UE4 when the scene view tears down, so we need to instantiate a new one every frame.
    // 没帧都需要创建up scaler
    return new FQxSpatialUpscaler();
}
