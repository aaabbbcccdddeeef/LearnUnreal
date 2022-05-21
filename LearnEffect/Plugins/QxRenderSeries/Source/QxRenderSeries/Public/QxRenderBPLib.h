// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QxRenderBPLib.generated.h"

class UTextureRenderTarget2D;
class AActor;

/**
 * 
 */
UCLASS()
class QXRENDERSERIES_API UQxRenderBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "QxRender", meta = (WorldContext = "WorldContextObject"))
	static void DrawQxShaderTestToRT(
			UTextureRenderTarget2D* OutRenderTarget,
			AActor* InActor, FLinearColor InColor);
};