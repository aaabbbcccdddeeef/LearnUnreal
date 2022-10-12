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

public:
	// 用这个来讲eyeAdaptaion Texture 复制下来,主要是为了测试
	UPROPERTY(EditAnywhere, Category="QxPostProcess")
	UTextureRenderTarget2D* QxEyeAdaptationDump = nullptr;

	// #TODO  PostProcessAsse可能在主线程修改，而渲染线程访问，
	// 但现在先不处理线程同步的问题
	// 访问post prosset asset的锁，这个object的内容可能同时被游戏线程更改和渲染线程读取
	FCriticalSection PostAssetMutex;
private:
	TSharedPtr<FQxBloomSceneViewExtension, ESPMode::ThreadSafe> QxBloomSceneViewExtension;

	// #TODO 注意，这个对象再渲染线程中也有访问，需要实现同步
	// 现在来看，最好的方案是维护一份渲染线程的非UObject类型的拷贝，渲染线程访问拷贝的数据
	// 如果用标准的互斥锁、读写锁的方式怎样实现保护
	UPROPERTY(Transient)
	UQxBloomFlareAsset* PostProcessAsset;
};
