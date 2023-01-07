// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestGeometryActor.h"

#include "QxRenderSubsystem.h"
#include "RenderGraphBuilder.h"
#include "ShaderParameterStruct.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values
AQxTestGeometryActor::AQxTestGeometryActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxTestGeometryActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxTestGeometryActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UQxRenderSubsystem* QxRenderSubsystem = GEngine->GetEngineSubsystem<UQxRenderSubsystem>();
	check(QxRenderSubsystem);
	FTextureRenderTarget2DResource* RenderTarget2DResource =
		static_cast<FTextureRenderTarget2DResource*>(QxRenderSubsystem->QxTestRT->GameThread_GetRenderTargetResource());
	check(RenderTarget2DResource);
	ENQUEUE_RENDER_COMMAND(DrawTestGeometry)
	([this, RenderTarget2DResource](FRHICommandListImmediate& RHICmdList)
	{
		AddDrawTestGeometryPass_RenderThread(RHICmdList, RenderTarget2DResource);
	});
}

void AQxTestGeometryActor::AddDrawTestGeometryPass_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTarget2DResource* RenderTargetResource)
{
	// RenderTestGeometryWithRDG(RHICmdList, RenderTargetResource);
	RenderTestGeometryWithCommandList(RHICmdList, RenderTargetResource);
}

