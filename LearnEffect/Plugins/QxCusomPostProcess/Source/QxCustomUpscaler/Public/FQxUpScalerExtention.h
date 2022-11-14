// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxSpatialUpscaler.h"
#include "QxTemporalUpscaler.h"
#include "SceneViewExtension.h"

// 自定义Upscaler， 主要参考FSR的做法
class FQxUpscalerExtention : public FSceneViewExtensionBase
{
public:
	FQxUpscalerExtention(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
	{
		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;
		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
		{
			IConsoleVariable* CvarQxUpscalerEnable =  IConsoleManager::Get().FindConsoleVariable(TEXT("r.QxUpscaler.Enable"));
			
			return CvarQxUpscalerEnable->GetBool();
		};
		IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override {}
	
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
		override{};

	

	// virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	// #TODO 不知道为什么这个移动到cpp中无法编译通过
	void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override
	{
		// InViewFamily.SetTemporalUpscalerInterface()

		InViewFamily.SetTemporalUpscalerInterface(new FQxTemporalUpscaler());
		// bool IsTemporalUpscalingRequest = false;
		// for (int32 ViewIndex = 0; ViewIndex < InViewFamily.Views.Num(); ++ViewIndex)
		// {
		// 	const FSceneView* View = InViewFamily.Views[ViewIndex];
		// 	if (ensure(View))
		// 	{
		// 		IsTemporalUpscalingRequest |= (View->PrimaryScreenPercentageMethod == EPrimaryScreenPercentageMethod::TemporalUpscale);
		// 	}
		// }
		//
		// if (!IsTemporalUpscalingRequest)
		// {
		// 	InViewFamily.SetPrimarySpatialUpscalerInterface(new FQxSpatialUpscaler());
		// 	InViewFamily.SetSecondarySpatialUpscalerInterface(new FQxSpatialUpscaler());
		// }
		// else
		// {
		// 	InViewFamily.SetSecondarySpatialUpscalerInterface(new FQxSpatialUpscaler());
		// }
	}

	virtual void SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo) override {};
};


