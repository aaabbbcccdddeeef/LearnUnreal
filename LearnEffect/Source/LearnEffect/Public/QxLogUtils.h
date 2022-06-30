// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QxLogUtils.generated.h"

/**
 * 
 */
UCLASS()
class LEARNEFFECT_API UQxLogUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category="QxLog", meta=(Keywords="DrawMaterialToRenderTarget", WorldContext="WorldContextObject"))
	static void LogTest(UObject* WorldContextObject,  const FName& InLogName);
};
