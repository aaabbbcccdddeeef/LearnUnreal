#include "QxPostProcessDownSample.h"

#include <stdexcept>

#include "QxBloomSceneViewExtension.h"
#include "QxPostProcessBloom.h"

namespace 
{
	class FQxDownSamplePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxDownSamplePS);
		SHADER_USE_PARAMETER_STRUCT(FQxDownSamplePS, FGlobalShader);
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(FVector2D, InputSize)
		END_SHADER_PARAMETER_STRUCT()
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}		
	};
	IMPLEMENT_GLOBAL_SHADER(FQxDownSamplePS, "/QxPPShaders/Bloom/QxDownSample.usf", "DownSamplePS", SF_Pixel);

	// Bloom upsample + combine
	class FQxUpsampleCombinePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxUpsampleCombinePS);
		SHADER_USE_PARAMETER_STRUCT(FQxUpsampleCombinePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(FVector2D, InputSize)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, PreviousTexture)
			SHADER_PARAMETER(float, Radius)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxUpsampleCombinePS, "/QxPPShaders/QXUpsampleCombine.usf", "UpsampleCombinePS", SF_Pixel);
}

FRDGTextureRef FQxBloomSceneViewExtension::RenderDownSample(
	FRDGBuilder& GraphBuilder,
	const FString& PassName,
	const FViewInfo& View, FRDGTextureRef InputTexture,
	const FIntRect& Viewport)
{
	// Build Texture
	FRDGTextureDesc Desc = InputTexture->Desc;
	Desc.Reset();
	Desc.Extent = Viewport.Size();
	Desc.Format = PF_FloatRGB;
	Desc.ClearValue = FClearValueBinding(FLinearColor::Black);
	FRDGTextureRef TargetTexture = GraphBuilder.CreateTexture(Desc,
		*PassName);

	
	// Render Shader
	TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
	TShaderMapRef<FQxDownSamplePS> PixelShader(View.ShaderMap);

	FQxDownSamplePS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxDownSamplePS::FParameters>();
	PassParams->Pass.InputTexture = InputTexture;
	PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(
		TargetTexture,
		ERenderTargetLoadAction::ENoAction
		);
	PassParams->Pass.InputTextureSampler = BilinearBorderSampler;
	PassParams->InputSize = FVector2D(Viewport.Size());

	DrawShaderpass(
	GraphBuilder,
	PassName,
	PassParams,
	VertexShader,
	PixelShader,
	ClearBlendState,
	Viewport
	);

	return TargetTexture;
}

FRDGTextureRef FQxBloomSceneViewExtension::RenderUpsampleCombine(FRDGBuilder& GraphBuilder, const FString& PassName,
	const FViewInfo& View, const FScreenPassTexture& InputTexture, const FScreenPassTexture& PreviousTexture,
	float Radius)
{
	// Build texture
	FRDGTextureDesc Desc = InputTexture.Texture->Desc;
	Desc.Reset();
	Desc.Extent = InputTexture.ViewRect.Size();
	Desc.Format = PF_FloatRGB;
	Desc.ClearValue = FClearValueBinding(FLinearColor::Black);
	FRDGTextureRef TargetTexture = GraphBuilder.CreateTexture(Desc, *PassName);

	// setup shader params
	TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
	TShaderMapRef<FQxUpsampleCombinePS> PixelShader(View.ShaderMap);

	FQxUpsampleCombinePS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxUpsampleCombinePS::FParameters>();
	PassParams->Pass.InputTexture = InputTexture.Texture;
	PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(TargetTexture, ERenderTargetLoadAction::ENoAction);
	PassParams->Radius = Radius;
	PassParams->Pass.InputTextureSampler = BilinearClampSampler;
	PassParams->InputSize = FVector2D(PreviousTexture.ViewRect.Size());
	PassParams->PreviousTexture = PreviousTexture.Texture;

	// render shader
	DrawShaderpass(
		GraphBuilder,
		PassName,
		PassParams,
		VertexShader,
		PixelShader,
		ClearBlendState,
		InputTexture.ViewRect
		);

	return  TargetTexture;
}

