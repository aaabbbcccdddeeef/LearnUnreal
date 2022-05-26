// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxTestCloud.generated.h"

struct TestStruct
{
	FVector TestPosition;
};

UCLASS()
class QXRENDERSERIES_API AQxTestCloud : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AQxTestCloud();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "QxRenderSeries")
	void RenderCloud(UTextureRenderTarget2D* InRenderTarget);

private:

	void RenderCloud_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		ERHIFeatureLevel::Type FeatureLevel,
		FRHITexture* InRenderTarget,
		int32 sizeX,
		int32 sizeY
	);

	bool RenderCloud_RenderThreadInit = false;

	FTexture2DRHIRef TextureRHI;
	FUnorderedAccessViewRHIRef TextureUAV;

	FStructuredBufferRHIRef TestStructureBuff;
	FUnorderedAccessViewRHIRef TestStructureBuffUAV;
};
