// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxUIBPLib.generated.h"

/**
 * 
 */
UCLASS()
class LEARNUI_API UQxUIBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static TSharedPtr<FSlateUser> GetCurrentSlateUser(UObject* WorldContext);
};
