// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RHI.h"
#include "RHIResources.h"
#include "UObject/Object.h"

class UQxColorCorrectSubsystem;
/**
 * 
 */
class QXCOLORCORRECTION_API FQxColorCorrectSceneViewExtension : public FSceneViewExtensionBase
{
public:
 FQxColorCorrectSceneViewExtension(const FAutoRegister& AutoRegister, UQxColorCorrectSubsystem* InWorldSubsystem);

#pragma region FSceneViewExtensionBaseInterfaces
 //~ Begin FSceneViewExtensionBase Interface
 virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override{}
 virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override{}
 virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override{}
 virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override{}
 virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override{}
 virtual void PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override{}

 // 这个回调在引擎后期之前执行，先参照colorCorrectRegion插件在这里做，之后可以测试使用另一个回调在后期执行过程中执行的回调
 virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
 
 //~ End FSceneViewExtensionBase Interface
#pragma endregion

private:
 UQxColorCorrectSubsystem* QxColorCorrectSubsystem;
};
