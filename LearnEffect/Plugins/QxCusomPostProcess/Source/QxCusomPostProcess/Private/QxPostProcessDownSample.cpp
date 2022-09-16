#include "QxPostProcessDownSample.h"

#include <stdexcept>

#include "PixelShaderUtils.h"
#include "QxBloomSceneViewExtension.h"
#include "QxPostProcessBloom.h"


namespace 
{
	// 参考文档https://www.froyok.fr/blog/2021-12-ue4-custom-bloom/中COD的filter实现的
	class FQxCOD_DownSamplePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxCOD_DownSamplePS);
		SHADER_USE_PARAMETER_STRUCT(FQxCOD_DownSamplePS, FGlobalShader);
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(FVector2D, InputSize)
		END_SHADER_PARAMETER_STRUCT()
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}		
	};
	IMPLEMENT_GLOBAL_SHADER(FQxCOD_DownSamplePS, "/QxPPShaders/QxDownSample.usf", "DownSamplePS", SF_Pixel);

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
	IMPLEMENT_GLOBAL_SHADER(FQxUpsampleCombinePS, "/QxPPShaders/QxUpsampleCombine.usf", "UpsampleCombinePS", SF_Pixel);
}

FRDGTextureRef FQxBloomSceneViewExtension::RenderCOD_DownSample(
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
	TShaderMapRef<FQxCOD_DownSamplePS> PixelShader(View.ShaderMap);

	FQxCOD_DownSamplePS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxCOD_DownSamplePS::FParameters>();
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
	if (!Desc.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Test"));
	}
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

namespace QxRenderUtils
{

	FQxDownsampleParameters GetDownSampleParameter(
		const FViewInfo& View,
		FScreenPassTexture Output,
		const FScreenPassTexture Input,
		EQxDownampleQuality Quality
		)
	{
		check(Output.IsValid());
		check(Input.IsValid());

		const FScreenPassTextureViewportParameters InputParams =
			GetScreenPassTextureViewportParameters(FScreenPassTextureViewport(Input));
		const FScreenPassTextureViewportParameters OutputParams =
			GetScreenPassTextureViewportParameters(FScreenPassTextureViewport(Output));

		FQxDownsampleParameters Parameters;
		Parameters.ViewUniformBuffer = View.ViewUniformBuffer;
		Parameters.Input = InputParams;
		Parameters.Output = OutputParams;
		Parameters.InputTexture = Input.Texture;
		Parameters.InputTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp,AM_Clamp>::GetRHI();
		return Parameters;
	}
	
	FScreenPassTexture AddQxDownSamplePass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& ViewInfo,
	const FQxDownSampleInputs& DownSampleInputs)
	{

		bool bIsComputePass = false;

		FScreenPassRenderTarget Output;
		// 创建一个作为render target的texture
		{
			FRDGTextureDesc TexDesc = DownSampleInputs.SceneColor.Texture->Desc;
			TexDesc.Reset();
			TexDesc.Extent = FIntPoint::DivideAndRoundUp(TexDesc.Extent, 2);
			TexDesc.Extent.X = FMath::Max(1, TexDesc.Extent.X);
			TexDesc.Extent.Y = FMath::Max(1, TexDesc.Extent.Y);
			TexDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_UAV);
			TexDesc.Flags |= bIsComputePass ? TexCreate_UAV : TexCreate_RenderTargetable;
			// TexDesc.Flags |= GFastVRamConfig.Downsample;
			TexDesc.ClearValue = FClearValueBinding(FLinearColor(0, 0, 0, 0));

			if (DownSampleInputs.FormatOverride != PF_Unknown)
			{
				TexDesc.Format = DownSampleInputs.FormatOverride;
			}

			Output.Texture = GraphBuilder.CreateTexture(TexDesc, DownSampleInputs.Name);
			Output.ViewRect = FIntRect::DivideAndRoundUp(DownSampleInputs.SceneColor.ViewRect, 2);
			Output.LoadAction = ERenderTargetLoadAction::ENoAction;
		}

		const FScreenPassTextureViewport OutputViewport(Output);

		// #TODO compute shader实现
		
		FQxNormalDownSamplePS::FPermutationDomain PermutationVector;
		PermutationVector.Set<FQxNormalDownSamplePS::FDownsampleQualityDimension>(DownSampleInputs.Quality);

		FQxNormalDownSamplePS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxNormalDownSamplePS::FParameters>();
		PassParams->Common = QxRenderUtils::GetDownSampleParameter(ViewInfo, Output,DownSampleInputs.SceneColor, DownSampleInputs.Quality);
		PassParams->RenderTargets[0] = Output.GetRenderTargetBinding();
		
		TShaderMapRef<FQxNormalDownSamplePS> PixelShader(ViewInfo.ShaderMap, PermutationVector);
		
		FPixelShaderUtils::AddFullscreenPass(
			GraphBuilder,
			ViewInfo.ShaderMap,
			RDG_EVENT_NAME("QxDownsample %s %dx%d", DownSampleInputs.Name, DownSampleInputs.SceneColor.ViewRect.Width(), DownSampleInputs.SceneColor.ViewRect.Height()),
			PixelShader,
			PassParams,
			OutputViewport.Rect
			);

		return MoveTemp(Output);
	}
}



void FQxDownSampleChain::Init(FRDGBuilder& GraphBuilder,
                              const FViewInfo& View,
                              const FQxEyeAdaptationParameters& EyeAdaptationParameters,
                              const FScreenPassTexture& HalfResolutionSceneColor,
                              EQxDownampleQuality DownampleQuality,
                              bool bLogLumaInAlpha)
{
	check(HalfResolutionSceneColor.IsValid())
	RDG_EVENT_SCOPE(GraphBuilder, "QxSceneDownSample");
	static const TCHAR* PassNames[StageCount] =
	{
		nullptr,
		TEXT("Scene(1/4)"),
		TEXT("Scene(1/8)"),
		TEXT("Scene(1/16)"),
		TEXT("Scene(1/32)"),
		TEXT("Scene(1/64)")
	};
	static_assert(UE_ARRAY_COUNT(PassNames) == StageCount, "Passname size must match stage count");

	// 第一个是输入的half res scene color
	Textures[0] = HalfResolutionSceneColor;
	
	for (uint32 StageIndex = 1; StageIndex < StageCount; ++StageIndex)
	{
		const uint32 PreviousStageIndex = StageIndex - 1;
		
		FQxDownSampleInputs DownSampleInputs;
		DownSampleInputs.Name = PassNames[StageIndex];
		DownSampleInputs.Quality = DownampleQuality;
		DownSampleInputs.SceneColor = Textures[PreviousStageIndex];
		
		Textures[StageIndex] =  QxRenderUtils::AddQxDownSamplePass(
			GraphBuilder,
			View,
			DownSampleInputs);

		if (bLogLumaInAlpha)
		{
			bLogLumaInAlpha = false;

			Textures[StageIndex] = QxRenderUtils::AddBasicEyeAdaptationSetupPass(
				GraphBuilder,
				View,
				EyeAdaptationParameters,
				Textures[StageIndex]
				);
		}
	}

	bIsInitilized = true;
}

