// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QxDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class QXASSETS_API UQxDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "QxAssets")
	FString Test1;

	UPROPERTY(EditAnywhere, Category = "QxAssets")
	float Test2;

	UPROPERTY(EditAnywhere, Category = "QxAssets")
	int32 test3;
};
