// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RenderGraphBuilder.h"
#include "RenderGraph.h"
// #include "ScreenPass.h"
#include "Renderer/Private/ScreenPass.h"
#include "UObject/Object.h"

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

				// SetShaderParameters(RHICmdList,
				// 	PixelShader,
				// 	PixelShader.GetShader(),
				// 	*PassParameters
				// 	);
				//
				// DrawRectangle(
				// 	RHICmdList,
				// 	0.f, 0.f,
				// 	Viewport.Width(), Viewport.Height(),
				// 	Viewport.Min.X, Viewport.Min.Y,
				// 	Viewport.Width(), Viewport.Height(),
				// 	Viewport.Size(),
				// 	Viewport.Size(),
				// 	PipelineState.VertexShader,
				// 	EDrawRectangleFlags::EDRF_Default
				// 	);
			}
			);
	}
}
