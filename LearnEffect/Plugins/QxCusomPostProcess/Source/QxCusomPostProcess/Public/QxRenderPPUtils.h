#pragma once
#include "ScreenPass.h"

namespace QxRenderPPUtils
{
	extern  FScreenPassTexture RenderKawaseBlur(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FScreenPassTexture& InputTexture,
		int32 BlurSteps);




// the vertext shader to draw a rectange
class FQxScreenPassVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FQxScreenPassVS);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&)
	{
		return true;
	}

	FQxScreenPassVS() = default;
	FQxScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};



template<typename TShaderParameters, typename  TShaderClassVertex, typename TShaderClassPixel>
void QxDrawScreenPass(
	FRDGBuilder& GraphBuilder,
	const FString& PassName,
	TShaderParameters* PassParameters,
	TShaderMapRef<TShaderClassVertex> VertexShader,
	TShaderMapRef<TShaderClassPixel> PixelShader,
	FRHIBlendState* BlendState,
	const FIntRect& Viewport
	)
{
	const FScreenPassPipelineState PipelineState(VertexShader, PixelShader, BlendState);
	GraphBuilder.AddPass(
		FRDGEventName(TEXT("%s"), *PassName),
		PassParameters,
		ERDGPassFlags::Raster,
		[PixelShader, PassParameters, Viewport, PipelineState](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(Viewport.Min.X, Viewport.Min.Y, 0.f,
				Viewport.Max.X, Viewport.Max.Y, 1.f);
			SetScreenPassPipelineState(RHICmdList, PipelineState);

			SetShaderParameters(
				RHICmdList,
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


template<typename TShaderParameters,typename TShaderClassPixel>
void QxDrawScreenPass(
		FRDGBuilder& GraphBuilder,
		const FString& PassName,
		TShaderMapRef<TShaderClassPixel> PixelShader,
		TShaderParameters* PassParameters,
		const FIntRect& Viewport,
		FRHIBlendState* InBlendState = nullptr
	)
{
	TShaderMapRef<FQxScreenPassVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FRHIBlendState* BlendState = InBlendState ? InBlendState : TStaticBlendState<>::GetRHI();
	QxDrawScreenPass(
		GraphBuilder,
		PassName,
		PassParameters,
		VertexShader,
		PixelShader,
		BlendState,
		Viewport
		);
}
}