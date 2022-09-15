﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "QxBloomSceneViewExtension.h"

#include <stdexcept>

#include "QxLensFlareAsset.h"
#include "QxPostProcessBloom.h"
#include "QxPostprocessSubsystem.h"
#include "ScreenPass.h"
#include "PostProcess/PostProcessMaterial.h"

FQxBloomSceneViewExtension::FQxBloomSceneViewExtension(const FAutoRegister& AutoRegister,
	UQxPostprocessSubsystem* InSubsystem)
		: FSceneViewExtensionBase(AutoRegister), QxPostprocessSubsystem(InSubsystem)
{
	check(QxPostprocessSubsystem);

	InitRenderStates();
}

FQxBloomSceneViewExtension::~FQxBloomSceneViewExtension()
{
	QxPostprocessSubsystem = nullptr;
	ClearBlendState = nullptr;
	AdditiveBlendState = nullptr;

	BilinearBorderSampler = nullptr;
	BilinearClampSampler = nullptr;
	BilinearRepeatSampler = nullptr;
}

void FQxBloomSceneViewExtension::InitRenderStates()
{
	if (ClearBlendState)
	{
		return;
	}

	// Blend modes from:
	// '/Engine/Source/Runtime/RenderCore/Private/ClearQuad.cpp'
	// '/Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessMaterial.cpp'
	ClearBlendState = TStaticBlendState<>::GetRHI();
	AdditiveBlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();

	BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	BilinearBorderSampler = TStaticSamplerState<SF_Bilinear, AM_Border, AM_Border, AM_Border>::GetRHI();
	BilinearRepeatSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
	NearestRepeatSampler = TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
}



void FQxBloomSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId,
                                                               FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	// FSceneViewExtensionBase::SubscribeToPostProcessingPass(Pass, InOutPassCallbacks, bIsPassEnabled);
	// 当前来看插入到这个pass之后是最合适的
	if (PassId == EPostProcessingPass::MotionBlur)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(
			this, &FQxBloomSceneViewExtension::RenderQxBloom_RenderThread));
	}
}

FScreenPassTexture FQxBloomSceneViewExtension::RenderQxBloom_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View, const FPostProcessMaterialInputs& PPMaterialInputs)
{
	if (nullptr == QxPostprocessSubsystem->GetBloomSettingAsset() ||
		!QxPostprocessSubsystem->GetBloomSettingAsset()->bEnableQxPPEffect)
	{
		return FScreenPassTexture(PPMaterialInputs.GetInput(EPostProcessMaterialInput::SceneColor));
		// return FScreenPassTexture();
	}
	// UE_LOG(LogTemp, Warning, TEXT("tEST Qxbloom"));
	//
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
	FScreenPassTexture BloomTexture = RenderBloomFlare(
		GraphBuilder,
		ViewInfo,
		PPMaterialInputs,
		QxPostprocessSubsystem->GetBloomSettingAsset());

	DownSampleTextures.Empty();
	UpSampleTextures.Empty();

	FScreenPassTexture HalResSceneColor = AddDownSamplePass(
		GraphBuilder,
		ViewInfo,
		PPMaterialInputs
		);
	
	FScreenPassTexture EyeAdaptationTexture = RenderEyeAdaptation(
		GraphBuilder,
		ViewInfo,
		PPMaterialInputs
		);
	
	
	// #TODO
	// My Tone Mapping
	return BloomTexture;
}

