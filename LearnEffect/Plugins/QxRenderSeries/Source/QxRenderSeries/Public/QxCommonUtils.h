// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxCommonUtils.generated.h"

UCLASS()
class QXRENDERSERIES_API UQxCommonUtils : public UBlueprintFunctionLibrary
{
 GENERATED_BODY()

#pragma region IterateActorOrComponents

 UFUNCTION(BlueprintCallable)
 static void TraverseActor(UObject* WorldContext);

 UFUNCTION(BlueprintCallable)
 static void GetComponents(AActor* InActor);
#pragma endregion
};



