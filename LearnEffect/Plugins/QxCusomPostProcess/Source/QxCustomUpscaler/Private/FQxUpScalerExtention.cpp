// Fill out your copyright notice in the Description page of Project Settings.


#include "FQxUpScalerExtention.h"

#include "QxSpatialUpscaler.h"


static TAutoConsoleVariable<bool> CVarQxUpscalerEnable(
    TEXT("r.QxUpscaler.Enable"),
    false,
    TEXT("Wheather use qx upscaler "),
    ECVF_Default);

static TAutoConsoleVariable<int> CVarQxAutomationViewIndex(
    TEXT("r.QxRender.Automation.ViewIndex"),
    0,
    TEXT("Select which view to use with NGX DLSS image quality and performance automation. (default = 0). \n"),
    ECVF_RenderThreadSafe);



// void FQxUpscalerExtention::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList,
//                                                             FSceneViewFamily& InViewFamily)
// {
//     int32 ViewIndex = CVarQxAutomationViewIndex.GetValueOnRenderThread();
//
//     if (InViewFamily.Views.IsValidIndex(ViewIndex))
//     {
//         const FSceneView* View = InViewFamily.Views[ViewIndex];
//         if (View->bIsGameView )
//         {
//             const FString AiAgentMarker = FString::Printf(TEXT("{\"camera\":{\"position\":{\"x\": %f, \"y\": %f, \"z\": %f},\"rotation\":{\"pitch\": %f, \"roll\": %f, \"yaw\": %f}}}"),
//                                                           View->ViewLocation.X, View->ViewLocation.Y, View->ViewLocation.Z,
//                                                           View->ViewRotation.Pitch, View->ViewRotation.Roll, View->ViewRotation.Yaw);
//
//             static FColor ColorMarker = FColor::FromHex("0xA1A5E87");
//             RHICmdList.PushEvent(*AiAgentMarker, ColorMarker);
//             RHICmdList.PopEvent();
//         }
//     }
// }

