// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxPostprocessSubsystem.generated.h"

// DECLARE_MULTICAST_DELEGATE_FourParams( FPP_LensFlares, FRDGBuilder&, const FViewInfo&, const FLensFlareInputs&, FLensFlareOutputsData& );
// extern RENDERER_API FPP_LensFlares PP_LensFlares;

class UQxBloomFlareAsset;
class FQxBloomSceneViewExtension;
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

	UQxBloomFlareAsset* GetBloomSettingAsset() const
	{
		return PostProcessAsset;
	}
private:
	TSharedPtr<FQxBloomSceneViewExtension, ESPMode::ThreadSafe> QxBloomSceneViewExtension;

	UPROPERTY(Transient)
	UQxBloomFlareAsset* PostProcessAsset;

	// #TODO  PostProcessAsse可能在主线程修改，而渲染线程访问，
	// 但现在先不处理线程同步的问题
	FCriticalSection RegionAccessCriticalSection;
};
