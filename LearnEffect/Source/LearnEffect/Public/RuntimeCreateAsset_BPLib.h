// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RuntimeCreateAsset_BPLib.generated.h"

/**
 * 
 */
UCLASS()
class LEARNEFFECT_API URuntimeCreateAsset_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Test")
	static void CreateTestTexture2();

	UFUNCTION(BlueprintCallable, Category = "Test")
		static void CreateTestTexture3();
};
