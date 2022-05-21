// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderBPLib.h"
#include "QxRenderSeries.h"
#include <Engine/TextureRenderTarget2D.h>

void RenderMyTest1(FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutRTResource,
	ERHIFeatureLevel::Type InFeatureLevel,
	FLinearColor InColor)
{
	FGlobalShaderMap* globalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);


}

void DrawQxShaderTestToRT_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutRTResource,
	ERHIFeatureLevel::Type InFeatureLevel,
	FLinearColor InColor)
{
	check(IsInRenderingThread());

	FRHITexture2D* rtRHITexture = OutRTResource->GetRenderTargetTexture();

	RHICmdList.Transition(FRHITransitionInfo(rtRHITexture, ERHIAccess::SRVMask, ERHIAccess::RTV));

	FRHIRenderPassInfo rpInfo(rtRHITexture, ERenderTargetActions::DontLoad_Store);

#pragma region MyRegion

#pragma endregion


	RHICmdList.BeginRenderPass(rpInfo, TEXT("QxShaderTest"));
	RHICmdList.EndRenderPass();
	RHICmdList.Transition(FRHITransitionInfo(rtRHITexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
}



void UQxRenderBPLib::DrawQxShaderTestToRT(
	UTextureRenderTarget2D* OutRenderTarget, 
	AActor* InActor, 
	FLinearColor InColor)
{
	check(IsInGameThread());

	if (!(OutRenderTarget && InActor))
	{
		UE_LOG(QxRender, Warning, TEXT("content from function %s"));
		return;
	}

	FTextureRenderTargetResource* rtResource =
		OutRenderTarget->GameThread_GetRenderTargetResource();

	UWorld* world = InActor->GetWorld();
	ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(QxDrawTest)(
		[rtResource, featureLevel, InColor](FRHICommandListImmediate& RHICmdList)
		{
			DrawQxShaderTestToRT_RenderThread(RHICmdList, rtResource, featureLevel, InColor);
		}
		);
}
