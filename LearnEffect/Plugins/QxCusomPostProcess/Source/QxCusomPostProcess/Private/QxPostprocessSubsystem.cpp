// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPostprocessSubsystem.h"

#include "QxBloomSceneViewExtension.h"
#include "QxLensFlareAsset.h"
#include "SceneViewExtension.h"

void UQxPostprocessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	QxBloomSceneViewExtension = FSceneViewExtensions::NewExtension<FQxBloomSceneViewExtension>(this);

	//加载Bloom Flare setting object asset

	FString AssetPath = "QxBloomFlareAsset'/QxCusomPostProcess/DA_QxBloomFlareDefault.DA_QxBloomFlareDefault'";
	PostProcessAsset = LoadObject<UQxBloomFlareAsset>(nullptr, *AssetPath);
	checkf(PostProcessAsset, TEXT("Does not find QxBloomFlare setting asset"));
}

void UQxPostprocessSubsystem::Deinitialize()
{
	// 这段参考自ColorCorrectionRegionSubsystem.cpp
	// 按照其注释是防止ScenViewExtension在别的地方被强引用，但还是不太理解 #TODO
	{
		QxBloomSceneViewExtension->IsActiveThisFrameFunctions.Empty();

		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
		{
			return TOptional<bool>(false);
		};

		QxBloomSceneViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}
	
	QxBloomSceneViewExtension.Reset();
	QxBloomSceneViewExtension = nullptr;

	PostProcessAsset = nullptr;
	Super::Deinitialize();
}
