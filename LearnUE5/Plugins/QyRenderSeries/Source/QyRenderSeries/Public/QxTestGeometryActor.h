// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxTestGeometryActor.generated.h"



/**
 * @brief 这个类主要用来测试在Tick过程中往渲染线程插入渲染自定义图形的RenderPass，不走UE的mesh draw pipeline
 * 这样做其实实践中可能涉及渲染线程游戏线程对象的生命周期问题，测试先不考虑
 */
UCLASS()
class QYRENDERSERIES_API AQxTestGeometryActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxTestGeometryActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	void AddDrawTestGeometryPass_RenderThread(FRHICommandListImmediate& RHICmdList,
		FTextureRenderTarget2DResource* RenderTargetResource);
};
