// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RenderGraphBuilder.h"
#include "SceneViewExtension.h"
#include "RHI.h"
#include "RHIResources.h"
#include "SceneRendering.h"
#include "UObject/Object.h"


class UQxBloomFlareAsset;
class UQxPostprocessSubsystem;




class FQxBloomSceneViewExtension : public FSceneViewExtensionBase
{
public:
	FQxBloomSceneViewExtension(const FAutoRegister& AutoRegister, UQxPostprocessSubsystem* InSubsystem);
	~FQxBloomSceneViewExtension();

#pragma region FSceneViewExtensionBaseInterfaces
	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override{}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override{}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override{}
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override{}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override{}
	virtual void PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override{}

	// 这个回调在引擎后期之前执行，先参照colorCorrectRegion插件在这里做，之后可以测试使用另一个回调在后期执行过程中执行的回调
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override{};

	virtual void SubscribeToPostProcessingPass(EPostProcessingPass Pass, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;
	//~ End FSceneViewExtensionBase Interface
	#pragma endregion

private:

	FScreenPassTexture RenderQxBloom_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& PPMaterialInputs);
	
	FScreenPassTexture RenderBloomFlare(FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FPostProcessMaterialInputs& PostProcessMaterialInput,
		const UQxBloomFlareAsset* QxBloomSettingAsset);
	
	FScreenPassTexture RenderEyeAdaptation(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FPostProcessMaterialInputs& PostProcessMaterialInput);

	FScreenPassTexture AddDownSamplePass(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FPostProcessMaterialInputs& PostProcessMaterialInput
		);
	
	FScreenPassTexture RenderFlare(FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FPostProcessMaterialInputs& PostProcessMaterialInput,
		FScreenPassTexture InputTexture
		);

	FScreenPassTexture RenderBloom(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FScreenPassTexture SceneColorTexture,
		int32 PassAmount
		);

	// 这个是参照文档中COD的filter的特殊down sample
	FRDGTextureRef RenderCOD_DownSample(FRDGBuilder& GraphBuilder,
		const FString& PassName,
		const FViewInfo& View,
		FRDGTextureRef InputTexture,
		const FIntRect& Viewport);

	FRDGTextureRef RenderUpsampleCombine(
		FRDGBuilder& GraphBuilder,
		const FString& PassName,
		const FViewInfo& View,
		const FScreenPassTexture& InputTexture,
		const FScreenPassTexture& PreviousTexture,
		float Radius
		);
	
	void InitRenderStates();

private:
	UQxPostprocessSubsystem* QxPostprocessSubsystem;
	

	// scene color 依次 downsample得到的结果
	TArray<FScreenPassTexture> DownSampleTextures;

	// downsample upsample得到的结果
	TArray<FScreenPassTexture> UpSampleTextures;
	
	// 保存bloom 和flare 设置的asset
	// UQxBloomFlareAsset* QxBloomSettingAsset;

	// Helpers
	// internal blending and sampler states
	FRHIBlendState* ClearBlendState  = nullptr;
	FRHIBlendState* AdditiveBlendState = nullptr;

	FRHISamplerState* BilinearClampSampler = nullptr;
	FRHISamplerState* BilinearBorderSampler = nullptr;
	FRHISamplerState* BilinearRepeatSampler = nullptr;
	FRHISamplerState* NearestRepeatSampler = nullptr;
};
