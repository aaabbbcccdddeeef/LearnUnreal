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

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static void SetTextureForamt(UTexture2D* InTexture, int32 InSizeX, int32 InSizeY);
	
	UFUNCTION(BlueprintCallable, Category = "QxRender", meta = (WorldContext = "WorldContextObject"))
	static void DrawCheckBoard(const UObject* WorldContextObject,
		UTextureRenderTarget2D* OutRenderTarget);

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static bool GetUseD3D12InGame();

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static void SetUseD3D12InGame(bool InUseD3D12);

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static void TestAccessEngineGlobals(float& OutAverageFPS, float& OutAverageMS);
	
	static void TestAccessSubsystem(UObject* WorldContextObject);
	
	/**
	 * @brief 测试用 RDG实现的Compute Shader处理Inrender target
	 * @param InRenderTarget 
	 * @param InVertexPositions 用来选择生成颜色
	 */
	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static void RenderTexture_WithCSRDG(UTextureRenderTarget2D* InRenderTarget, TArray<FVector> InVertexPositions);

	UFUNCTION(BlueprintCallable, Category = "QxRender")
	static TArray<FVector> GetMeshVerticesWS(UStaticMeshComponent* InMeshComponent);

	static void PostResolveSceneColor_RenderThread(FRHICommandListImmediate& FrhiCommandListImmediate,
		FSceneRenderTargets& SceneRenderTargets);

	static void PostClear_RenderThread(FRHICommandListImmediate& FrhiCommandListImmediate,
		FSceneRenderTargets& SceneRenderTargets);

	static   FSceneRenderTargetItem& RequestSurface(FRHICommandListImmediate& RHICmdList);


	static  TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
};
