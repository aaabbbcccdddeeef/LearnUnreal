// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestAssetLoad.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"



void UQxTestAssetLoad::TestLoadAssets()
{
	TArray<FSoftObjectPath> ToStream;

	for (auto& SomePtr : MyAssets)
	{
		ToStream.Add(SomePtr.ToSoftObjectPath());
	}

	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	StreamableManager.RequestAsyncLoad(
		ToStream,
		FStreamableDelegate::CreateUObject(
			this,
			&UQxTestAssetLoad::OnAssetLoad
			)
		);
}

void UQxTestAssetLoad::OnAssetLoad()
{
	UE_LOG(LogTemp, Warning, TEXT("Test asset load callback is invoked"));
}

