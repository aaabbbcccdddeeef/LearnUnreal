// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RenderGraphBuilder.h"
#include "RenderGraph.h"
// #include "ScreenPass.h"
#include "Renderer/Private/ScreenPass.h"
#include "UObject/Object.h"


//定义命令名称，其中STATGROUP_QxTestGroup是stat命令输入的文字， TEXT内是显示的，STATCAT_Advanced是固定写法
DECLARE_STATS_GROUP(TEXT("QxRender"), STATGROUP_QxRender, STATCAT_Advanced);

/**
 * 
 */
class QXRENDERSERIES_API FQxRenderUtils 
{
public:
	static TArray<FVector> GetVertexPositonsWS(UStaticMeshComponent* InMeshComponent);

	static bool RayCastHit(const FVector& RayOrigin, const FVector& RayDirection,
		float RayMarchingLength,  FHitResult& OutHitResult, AActor* InActor);
};


namespace QxRenderUtils
{

	template<typename TShaderParameters, typename  TShaderClassVertex, typename  TShaderClassPixel>
	void DrawShaderPass(
		FRDGBuilder& GraphBuilder,
		const FString& PasName,
		TShaderParameters* PassParameters,
		TShaderMapRef<TShaderClassVertex> VertexShader,
		TShaderMapRef<TShaderClassPixel> PixelShader,
		FRHIBlendState* BlendState,
		const FIntRect& Viewport
		)
	{
		const FScreenPassPipelineState PipelineState(VertexShader, PixelShader, BlendState);
		GraphBuilder.AddPass(
			FRDGEventName(TEXT("%s"), *PasName),
			PassParameters,
			ERDGPassFlags::Raster,
			[PixelShader, PassParameters, Viewport, PipelineState](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.SetViewport(Viewport.Min.X, Viewport.Min.Y, 0.f,
					Viewport.Max.X, Viewport.Max.Y, 1.f);
				SetScreenPassPipelineState(RHICmdList, PipelineState);

				SetShaderParameters(RHICmdList,
					PixelShader,
					PixelShader.GetPixelShader(),
					*PassParameters
					);
				
				DrawRectangle(
					RHICmdList,
					0.f, 0.f,
					Viewport.Width(), Viewport.Height(),
					Viewport.Min.X, Viewport.Min.Y,
					Viewport.Width(), Viewport.Height(),
					Viewport.Size(),
					Viewport.Size(),
					PipelineState.VertexShader,
					EDrawRectangleFlags::EDRF_Default
					);
			}
			);
	}


	void RenderAverageToTarget_RenderThread(FRHICommandListImmediate& RHICmdList, UTexture2D* InTexture,
		UTextureRenderTarget2D* InRenderTarget);
}

// 这个namespace 的方法不是直接用的，用来作为实现的参考
namespace QxRenderReference
{

	void GetRenderStates();

#pragma region TextureOperations_RenderThread

	FRDGTextureRef CreateRDGTexture(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture);
#pragma endregion
}
