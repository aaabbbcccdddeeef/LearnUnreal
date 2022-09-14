// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxLensFlareAsset.generated.h"

// 这个结构用来更方便的设置ghost 的设置
USTRUCT(BlueprintType)
struct FLensFlareGhostSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exedre")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exedre")
	float Scale = 1.0f;
};

/**
 * 
 */
UCLASS()
class QXCUSOMPOSTPROCESS_API UQxBloomFlareAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="General")
	bool bEnableQxPPEffect = true;
	
	// 这个主要用来控制bloom upsample combine 过程中，低分辨率和高分辨率的混合插值的参数
	UPROPERTY(EditAnywhere, Category="Bloom", meta=(UIMin = "0.0", UIMax = "1.0"))
	float BloomRadius = 0.3f;

	// 用来控制bloom过程的downsample 次数
	UPROPERTY(EditAnywhere, Category="Bloom", meta=(UIMin = "0.0", UIMax = "10"))
	int32 DownSampleCount = 8;

	UPROPERTY(EditAnywhere, Category="Bloom", meta=(UIMin="0.0", UIMax="1.0"))
	float BloomIntensity = 1.0f;
	
	UPROPERTY(EditAnywhere, Category="General", meta=(UIMin="0.0", UIMax="10.0"))
	float FlareIntensity = 1.0f;

	UPROPERTY(EditAnywhere, Category="General")
	FLinearColor FlareTint = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);

	UPROPERTY(EditAnywhere, Category="General")
	UTexture2D* FlareGradient = nullptr;

	UPROPERTY(EditAnywhere, Category="Threshold", meta=(UIMin="0.0", UIMax="10.0"))
	float ThresholdLevel = 1.0f;

	UPROPERTY(EditAnywhere, Category="Threshold", meta=(UIMin="0.01", UIMax="10.0"))
	float ThresholdRange = 1.0f;

	UPROPERTY(EditAnywhere, Category="Ghosts", meta=(UIMin="0.0", UIMax="1.0"))
	float GhostIntensity = 1.0f;

	UPROPERTY(EditAnywhere, Category="Ghosts", meta=(UIMin="0.0", UIMax="1.0"))
	float GhostChromaShift = 0.015f;

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost1 =  { FLinearColor(1.0f, 0.8f, 0.4f, 1.0f), -1.5f };

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost2 = { FLinearColor(1.0f, 1.0f, 0.6f, 1.0f),  2.5f };

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost3 =  { FLinearColor(0.8f, 0.8f, 1.0f, 1.0f), -5.0f };

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost4 =  { FLinearColor(0.5f, 1.0f, 0.4f, 1.0f), 10.0f };

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost5 =  { FLinearColor(0.5f, 0.8f, 1.0f, 1.0f),  0.7f };

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost6 =  { FLinearColor(0.9f, 1.0f, 0.8f, 1.0f), -0.4f};

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost7 =  { FLinearColor(1.0f, 0.8f, 0.4f, 1.0f), -0.2f };

	UPROPERTY(EditAnywhere, Category="Ghosts")
	FLensFlareGhostSettings Ghost8 =  { FLinearColor(0.9f, 0.7f, 0.7f, 1.0f), -0.1f };

	UPROPERTY(EditAnywhere, Category="Halo", meta=(UIMin = "0.0", UIMax = "1.0"))
	float HaloIntensity = 1.0f;

	UPROPERTY(EditAnywhere, Category="Halo", meta=(UIMin = "0.0", UIMax = "1.0"))
	float HaloWidth = 0.6f;

	UPROPERTY(EditAnywhere, Category="Halo", meta=(UIMin = "0.0", UIMax = "1.0"))
	float HaloMask = 0.5f;
	
	UPROPERTY(EditAnywhere, Category="Halo", meta=(UIMin = "0.0", UIMax = "1.0"))
	float HaloCompression = 0.65f;

	UPROPERTY(EditAnywhere, Category="Halo", meta=(UIMin = "0.0", UIMax = "1.0"))
	float HaloChromaShift = 0.015f;

	UPROPERTY(EditAnywhere, Category="Glare", meta=(UIMin = "0.0", UIMax = "10.0"))
	float GlareIntensity = 0.02f;

	UPROPERTY(EditAnywhere, Category="Glare", meta=(UIMin = "0.01", UIMax = "200.0"))
	float GlareDivider = 60.0f;

	UPROPERTY(EditAnywhere, Category="Glare", meta=(UIMin = "0.0", UIMax = "10.0"))
	FVector GlareScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Glare")
	FLinearColor GlareTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category="Glare")
	UTexture2D* GlareLineMask = nullptr;
};
