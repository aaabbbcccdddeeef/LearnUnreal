// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QxAssetBPLib.generated.h"

/**
 * 
 */
UCLASS()
class QXEDITORMODULE_API UQxAssetBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="QxAssetTool")
	static void Test1();

	UFUNCTION(BlueprintCallable, Category="QxAssetTool")
	static bool SetMeshLODWithAnotherMesh(UStaticMesh* InTargetMesh, UStaticMesh* InSrcMesh, int32 InLODIndex = 1); 
};
