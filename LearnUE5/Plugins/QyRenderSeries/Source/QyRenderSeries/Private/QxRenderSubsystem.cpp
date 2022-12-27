// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderSubsystem.h"

#include "QxTestRenderer.h"
#include "Engine/TextureRenderTarget2D.h"

void UQxRenderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FString SSROutputTexPath = "TextureRenderTarget2D'/QyRenderSeries/QxSSR/RT_QxSSR.RT_QxSSR'";
	QxSSROutput = LoadObject<UTextureRenderTarget2D>(nullptr, *SSROutputTexPath);

	ENQUEUE_RENDER_COMMAND(CreateQxTestRenderer)(
	[this](FRHICommandListImmediate& RHICmdList)
	{
		QxTestRenderer = MakeShared<FQxTestRenderer>(this);
	}
	);
}

void UQxRenderSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ENQUEUE_RENDER_COMMAND(CreateQxTestRenderer)(
	[this](FRHICommandListImmediate& RHICmdList)
	{
		QxTestRenderer.Reset();
	}
	);
}
