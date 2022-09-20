#pragma once
#include "ScreenPass.h"
#include "PostProcess/PostProcessMaterial.h"

class UQxBloomFlareAsset;

namespace QxPostProcess
{

	// FScreenPassTexture RenderBloomFlare(FRDGBuilder& GraphBuilder,
	// 	const FViewInfo& ViewInfo,
	// 	const FPostProcessMaterialInputs& PostProcessMaterialInput,
	// 	const UQxBloomFlareAsset* QxBloomSettingAsset);
}

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
inline void DrawShaderpass(
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