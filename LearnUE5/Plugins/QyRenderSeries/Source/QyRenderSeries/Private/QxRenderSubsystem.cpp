// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderSubsystem.h"

#include "QxTestRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"

namespace 
{

	
}

void UQxRenderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FString SSROutputTexPath = "TextureRenderTarget2D'/QyRenderSeries/QxSSR/RT_QxSSR.RT_QxSSR'";
	QxSSROutput = LoadObject<UTextureRenderTarget2D>(nullptr, *SSROutputTexPath);

	QxTestRT = CreateTestRT();

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

UTextureRenderTarget2D* UQxRenderSubsystem::CreateTestRT()
{
	UTextureRenderTarget2D* TestRT = NewObject<UTextureRenderTarget2D>(this);
	check(TestRT);
	TestRT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	TestRT->ClearColor = FLinearColor::Black;
	TestRT->bAutoGenerateMips = false;
	TestRT->InitAutoFormat(600, 400);
	TestRT->UpdateResourceImmediate(true);
	return TestRT;
}
