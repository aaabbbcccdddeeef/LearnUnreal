// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QxShaders.h"
#include "QxRenderBPLib.generated.h"

class UTextureRenderTarget2D;
class AActor;
class UTexture;

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
			AActor* InActor, FLinearColor InColor,
			UTexture* MyTexture,
			FMyUniformData InMyUniformData);

	UFUNCTION(BlueprintCallable, Category = "QxRender", meta = (WorldContext = "WorldContextObject"))
	static void TextureWriting(UTexture2D* TextureToWrite, AActor* selfref);

	UFUNCTION(BlueprintCallable, Category = "QxRender", meta = (WorldContext = "WorldContextObject"))
	static void TextureWriting2(UTexture2D* TextureToWrite, AActor* selfref);

	UFUNCTION(BlueprintCallable, Category = "QxRender", meta = (WorldContext = "WorldContextObject"))
	static void DrawCheckBoard(const UObject* WorldContextObject,
		UTextureRenderTarget2D* OutRenderTarget);

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static bool GetUseD3D12InGame();

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static void SetUseD3D12InGame(bool InUseD3D12);
};
