// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "RenderGraphBuilder.h"
// #include "SceneRendering.h"
#include "PostProcess/PostProcessLensFlares.h" 
#include "RendererInterface.h"
#include "UObject/Object.h"
#include "QxPostprocessSubsystem.generated.h"


class UQxLensFlareAsset;
DECLARE_MULTICAST_DELEGATE_FourParams(FPP_LensFlares, FRDGBuilder&, const FViewInfo&, const FLensFlareInputs&,
                                      FLensFlareOutputData&);
extern RENDERER_API FPP_LensFlares PP_LensFlares;


/**
 * 
 */
UCLASS()
class QXCUSOMPOSTPROCESS_API UQxPostprocessSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:

	// 保存设置的setting的引用
	UPROPERTY(Transient)
	UQxLensFlareAsset* PostprocessAsset;

	// 引擎中添加的delegate会调用这个
	void RenderLensFlare(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& View,
		const FLensFlareInputs& Inputs,
		FLensFlareOutputData& Outputs
		);

	// Threshold Render pass
	FRDGTextureRef RenderThreshold(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef InputTexture,
		FIntRect& InputRect,
		const FViewInfo& View
		);

	// Ghosts + Halo Render pass
	FRDGTextureRef RenderFlare(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef InputTexture,
		FIntRect& InputRect,
		const FViewInfo& View
		);

	// Glare Render pass
	FRDGTextureRef RenderGlare(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef InputTexture,
		FIntRect& InputRect,
		const FViewInfo& View
		);

	// sub-pass for bluring
	FRDGTextureRef RenderBlur(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef InputTexture,
		const FViewInfo& View,
		const FIntRect& Viewport,
		int BlurSteps
		);

	// cache blend states and sampling states
	// reused across render passes
	FRHIBlendState* ClearBlendState = nullptr;
	FRHIBlendState* AdditiveBlendState = nullptr;

	
	FRHISamplerState* BilinearClampSampler = nullptr;
	FRHISamplerState* BilinearBorderSampler = nullptr;
	FRHISamplerState* BilinearRepeatSampler = nullptr;
	FRHISamplerState* NearestRepeatSampler = nullptr;
	
};
