// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxTestAssetLoad.generated.h"

/**
 * 测试UE4 资源加载
 */
UCLASS(BlueprintType)
class LEARNUI_API UQxTestAssetLoad : public UDataAsset
{
	GENERATED_BODY()

public:
	void TestLoadAssets();
	
private:
	void OnAssetLoad();
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="QxTestAssetLoad")
	TSoftObjectPtr<UTexture2D> TestImagePtr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="QxTestAssetLoad")
	TArray< TSoftObjectPtr<UTexture2D> > MyAssets;
};
