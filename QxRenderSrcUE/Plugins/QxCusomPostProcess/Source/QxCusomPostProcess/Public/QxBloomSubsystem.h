// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScreenPass.h"
#include "UObject/Object.h"
#include "QxBloomSubsystem.generated.h"

class UQxLensFlareAsset;
// Add MDNobu
DECLARE_MULTICAST_DELEGATE_FourParams(FPP_CustomBloomFlare, FRDGBuilder&, const FViewInfo&,
	const FScreenPassTexture&, FScreenPassTexture&);
extern  RENDERER_API FPP_CustomBloomFlare PP_CustomFlare;

// 所有pass都要用到的rdg buffer input
BEGIN_SHADER_PARAMETER_STRUCT(FQxLensFlarePassParameters,)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

/**
 * 
 */
UCLASS()
class QXCUSOMPOSTPROCESS_API UQxBloomSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// Helpers
	// internal blending and sampler states
	FRHIBlendState* ClearBlendState  = nullptr;
	FRHIBlendState* AdditiveBlendState = nullptr;

	FRHISamplerState* BilinearClampSampler = nullptr;
	FRHISamplerState* BilinearBorderSampler = nullptr;
	FRHISamplerState* BilinearRepeatSampler = nullptr;
	FRHISamplerState* NearestRepeatSampler = nullptr;

	void InitStates();

	// Main function
	void Render(FRDGBuilder& GraphBuilder, const FViewInfo& View,
		const FScreenPassTexture& SceneColor, FScreenPassTexture& Output);
	
	TArray<FScreenPassTexture> MipmapsDownSample;
	TArray<FScreenPassTexture> MipMapUpSampler;

	// Bloom
	FScreenPassTexture RenderBloom(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& View,
		const FScreenPassTexture& SceneColor,
		int32 PassAmount
		);

	FRDGTextureRef RenderDownSample(
		FRDGBuilder& GraphBuilder,
		const FString& PassName,
		const FViewInfo& View,
		FRDGTextureRef InputTexture,
		const FIntRect& Viewport
		);

	FRDGTextureRef RenderUpsampleCombine(
		FRDGBuilder& GraphBuilder,
		const FString& PassName,
		const FViewInfo& View,
		const FScreenPassTexture& InputTexture,
		const FScreenPassTexture& PreviousTexture,
		float Radius
		);

	// 保存设置的setting的引用
	UPROPERTY(Transient)
	UQxLensFlareAsset* PostprocessAsset;
};
