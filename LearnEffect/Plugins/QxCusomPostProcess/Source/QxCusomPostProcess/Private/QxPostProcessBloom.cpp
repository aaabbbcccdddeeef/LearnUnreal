#include "QxPostProcessBloom.h"

#include <stdexcept>

#include "PixelShaderUtils.h"
#include "QxBloomSceneViewExtension.h"
#include "QxLensFlareAsset.h"
#include "QxPostprocessSubsystem.h"
#include "Interfaces/IPluginManager.h"
#include "QxRenderPPUtils.h"
#include "RenderGraph.h"
#include "ScreenPass.h"
#include "SystemTextures.h"

using QxRenderPPUtils::FQxScreenPassVS;

DECLARE_GPU_STAT(PostProcessQx);
TAutoConsoleVariable<int32> CVarQxBloomPassAmount(
TEXT("r.QxRender.BloomPassAmount"),
0,
TEXT("Number of passes to render bloom, override bloom asset setting"),
ECVF_RenderThreadSafe
);

TAutoConsoleVariable<float> CVarQxBloomRadius(
	TEXT("r.QxRender.BloomRadius"),
	0.f,
	TEXT("Size/Scale of the Bloom, override bloom asset setting if not 0")
	);
namespace
{
	class FQxMixPass : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxMixPass);
		SHADER_USE_PARAMETER_STRUCT(FQxMixPass, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BloomTexture)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GlareTexture)
			SHADER_PARAMETER_TEXTURE(Texture2D, FlareGradientTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, FlareGradientTextureSampler)
			SHADER_PARAMETER(FVector4, FlareTint)
			SHADER_PARAMETER(FVector2D, InputViewportSize)
			SHADER_PARAMETER(FVector2D, BufferSize)
			SHADER_PARAMETER(FVector2D, GlarePixelSize)
			SHADER_PARAMETER(FIntVector, MixPassFlags) //bloom/flare/glare是否开启的flag
			SHADER_PARAMETER(float, BloomIntensity)
			SHADER_PARAMETER(float, FlareIntensity)
		END_SHADER_PARAMETER_STRUCT()
		
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxMixPass, "/QxPPShaders/QxBloomMix.usf", "MixPS", SF_Pixel);

	class FQxFlareChromaPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxFlareChromaPS);
		SHADER_USE_PARAMETER_STRUCT(FQxFlareChromaPS, FGlobalShader);
			
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(float, ChromaShift)
		END_SHADER_PARAMETER_STRUCT()
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxFlareChromaPS, "/QxPPShaders/QxChroma.usf", "ChromaPS", SF_Pixel);

	class FQxFlareGhostPS : public  FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxFlareGhostPS);
		SHADER_USE_PARAMETER_STRUCT(FQxFlareGhostPS, FGlobalShader);
			
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER_ARRAY(FVector4, GhostColors, [8])
			SHADER_PARAMETER_ARRAY(float, GhostScales, [8])
			SHADER_PARAMETER(float, Intensity)
		END_SHADER_PARAMETER_STRUCT()
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxFlareGhostPS, "/QxPPShaders/QxGhosts.usf", "GhostPS", SF_Pixel);

	class FQxHaloPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxHaloPS);
		SHADER_USE_PARAMETER_STRUCT(FQxHaloPS, FGlobalShader);
			
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(float, Width)
			SHADER_PARAMETER(float, Mask)
			SHADER_PARAMETER(float, Compression)
			SHADER_PARAMETER(float, Intensity)
			SHADER_PARAMETER(float, ChromaShift)
		END_SHADER_PARAMETER_STRUCT()
		
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxHaloPS, "/QxPPShaders/QxHalo.usf", "HaloPS", SF_Pixel);

#pragma region GlareShaders
	class FQxGlareVS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxGlareVS);
		SHADER_USE_PARAMETER_STRUCT(FQxGlareVS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(FIntPoint, TileCount)
			SHADER_PARAMETER(FVector4, PixelSize)
			SHADER_PARAMETER(FVector2D, BufferSize)
		END_SHADER_PARAMETER_STRUCT()
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGlareVS, "/QxPPShaders/QxGlare.usf", "GlareVS", SF_Vertex);


	class FQxGlareGS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxGlareGS);
		SHADER_USE_PARAMETER_STRUCT(FQxGlareGS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER(FVector4, PixelSize)
			SHADER_PARAMETER(FVector2D, BufferSize)
			SHADER_PARAMETER(FVector2D, BufferRatio)
			SHADER_PARAMETER(float, GlareIntensity)
			SHADER_PARAMETER(float, GlareDivider)
			SHADER_PARAMETER(FVector4, GlareTint)
			SHADER_PARAMETER_ARRAY(float, GlareScales, [3])
		END_SHADER_PARAMETER_STRUCT()
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGlareGS, "/QxPPShaders/QxGlare.usf", "GlareGS", SF_Geometry);

	class FQxGlarePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxGlarePS);
		SHADER_USE_PARAMETER_STRUCT(FQxGlarePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_SAMPLER(SamplerState, GlareTextureSampler)
			SHADER_PARAMETER_TEXTURE(Texture2D, GlareTexture)
		END_SHADER_PARAMETER_STRUCT()
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGlarePS, "/QxPPShaders/QxGlare.usf", "GlarePS", SF_Pixel);
#pragma endregion
}




FScreenPassTexture FQxBloomSceneViewExtension::RenderBloomFlare(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FScreenPassTexture& SceneColor, const UQxBloomFlareAsset* QxBloomSettingAsset)
{
	bool bSupportHDROutput = GRHISupportsHDROutput;
	UE_LOG(LogTemp, Warning, TEXT("support hdr: %s"), (bSupportHDROutput ? TEXT("true") : TEXT("false")));
	
	// const FViewInfo& ViewInfo = dynamic_cast<FViewInfo>(View);
	RDG_GPU_STAT_SCOPE(GraphBuilder, PostProcessQx);
	RDG_EVENT_SCOPE(GraphBuilder, "PostProcessQx");

	const UQxBloomFlareAsset* BloomSettingAsset = QxPostprocessSubsystem->GetBloomSettingAsset();
	check(BloomSettingAsset);

	int32 DownSamplePassNum = BloomSettingAsset->DownSampleCount;
	if (CVarQxBloomPassAmount.GetValueOnRenderThread())
	{
		DownSamplePassNum = CVarQxBloomPassAmount.GetValueOnRenderThread();
	}

	const FScreenPassTexture BlackDummy
	{
		GraphBuilder.RegisterExternalTexture(
			GSystemTextures.BlackDummy,
			TEXT("BlackDummy")
			)
	};

	FScreenPassTexture BloomTexture;
	FScreenPassTexture FlareTexture;
	FScreenPassTexture GlareTexture;
	FScreenPassTexture SceneColorTexture =
		SceneColor;
	
	// Bloom
	{
		if (BloomSettingAsset->bEnableQxBloom)
		{
			BloomTexture = RenderBloom(
				GraphBuilder,
				ViewInfo,
				SceneColorTexture,
				DownSamplePassNum
				);
		}
	}	

	// Flare
	{
		FlareTexture = RenderFlare(
			GraphBuilder,
			ViewInfo,
			BloomTexture
			);
	}

	// Glare
	if (QxBloomSettingAsset->bEnableGlare)
	{
		GlareTexture = RenderGlare(
			GraphBuilder,
			ViewInfo,
			BloomTexture
			);
	}
	

	// Composite Bloom, Flare and Glare together

	
	FRDGTextureRef MixTexture;
	// FIntRect MixViewport(
	// 	0, 0,
	// 	ViewInfo.ViewRect.Width() / 2,
	// 	ViewInfo.ViewRect.Height() / 2
	// 	);
	// 这里应该用half 分辨率比较好，但由于插件的形式不太容易改tonemapping的输入，先用全分辨率
	FIntRect MixViewport(
		0, 0,
		ViewInfo.ViewRect.Width() ,
		ViewInfo.ViewRect.Height() 
		);
	// Render Mix Pass
	{
		RDG_EVENT_SCOPE(GraphBuilder, "MixPass");

		const FString PassName("Mix");

		float BloomIntensity = BloomSettingAsset->BloomIntensity;

		// If the internal blending for the upsample pass is additive
		// (aka not using the lerp) then uncomment this line to
		// normalize the final bloom intensity.
		//  BloomIntensity = 1.0f / float( FMath::Max( PassAmount, 1 ) );

		FVector2D BufferSize(
			float(MixViewport.Width()),
			float(MixViewport.Height())
			);

		FIntVector BufferValidity(
			(BloomTexture.IsValid()),
			(FlareTexture.IsValid()),
			(GlareTexture.IsValid())
			);

		// Create Texture
		FRDGTextureDesc Desc = SceneColorTexture.Texture->Desc;
		Desc.Reset();
		Desc.Extent = MixViewport.Size();
		Desc.Format = PF_FloatRGB;
		Desc.ClearValue = FClearValueBinding(FLinearColor::Black);
		MixTexture = GraphBuilder.CreateTexture(Desc, *PassName);

		// Render shader
		TShaderMapRef<FQxScreenPassVS> VertexShader(ViewInfo.ShaderMap);
		TShaderMapRef<FQxMixPass> PixelShader(ViewInfo.ShaderMap);

		FQxMixPass::FParameters* PassParams = GraphBuilder.AllocParameters<FQxMixPass::FParameters>();
		// 设置 shader 参数
		{
			PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(MixTexture, ERenderTargetLoadAction::ENoAction);
			PassParams->Pass.InputTextureSampler = BilinearClampSampler;
			PassParams->MixPassFlags = BufferValidity;
			// Bloom
			PassParams->BloomTexture = BlackDummy.Texture;
			PassParams->BloomIntensity = BloomIntensity;

			// Glare
			PassParams->GlareTexture = BlackDummy.Texture;
			PassParams->GlarePixelSize = FVector2D(1.f, 1.f) / BufferSize;

			// Flare
			PassParams->Pass.InputTexture  = BlackDummy.Texture;
			PassParams->FlareIntensity = BloomSettingAsset->FlareIntensity;
			PassParams->FlareTint = FVector4(BloomSettingAsset->FlareTint);
			PassParams->FlareGradientTexture = GWhiteTexture->TextureRHI;
			PassParams->FlareGradientTextureSampler = BilinearClampSampler;

			if (BloomSettingAsset->FlareGradient)
			{
				const FTextureRHIRef TextureRHI = BloomSettingAsset->FlareGradient->Resource->TextureRHI;
				PassParams->FlareGradientTexture = TextureRHI;
			}

			if (BufferValidity.X)
			{
				PassParams->BloomTexture = BloomTexture.Texture;
			}

			if (BufferValidity.Y)
			{
				PassParams->Pass.InputTexture = FlareTexture.Texture;
			}

			if (BufferValidity.Z)
			{
				PassParams->GlareTexture = GlareTexture.Texture;
			}
		}
		check(PassParams);
		ClearUnusedGraphResources(PixelShader, PassParams);
		
		// Render
		QxDrawScreenPass(
			GraphBuilder,
			PassName,
			PassParams,
			VertexShader,
			PixelShader,
			ClearBlendState,
			MixViewport
			);
	}
	
	// #TODO 先测试bloom
	FScreenPassTexture OutTexture;
	OutTexture.Texture = MixTexture;
	OutTexture.ViewRect = MixViewport;
	return OutTexture;
}


FScreenPassTexture FQxBloomSceneViewExtension::RenderBloom(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& ViewInfo,
	const FScreenPassTexture SceneColorTexture,
	int32 PassAmount)
{
	check(SceneColorTexture.IsValid());

	/// PassAmount <= 1 后面的upsample 等逻辑不适用
	if (PassAmount <= 1)
	{
		// return FScreenPassTexture();
		return SceneColorTexture;
	}
	RDG_EVENT_SCOPE(GraphBuilder, "BloomPass");

#pragma region DownSample
	const int32 Width = ViewInfo.ViewRect.Width();
	const int32 Height = ViewInfo.ViewRect.Height();
	int Divider = 2;

	FRDGTextureRef PreviousTexture = SceneColorTexture.Texture;

	for (int i = 0; i < PassAmount; ++i, Divider *= 2)
	{
		FIntRect Size(
			0, 0,
			FMath::Max(Width / Divider,1),
			FMath::Max(Height / Divider, 1)
			);

		const FString PassName = "Downsample"
								+ FString::FromInt(i)
								+ "_(1/"
								+ FString::FromInt( Divider )
								+ ")_"
								+ FString::FromInt( Size.Width() )
								+ "x"
								+ FString::FromInt( Size.Height() );

		FRDGTextureRef Texture = 
			RenderCOD_DownSample(
				GraphBuilder,
				PassName,
				ViewInfo,
				PreviousTexture,
				Size
				);

		FScreenPassTexture DownsampleTexture(Texture, Size);
		DownSampleTextures.Add(DownsampleTexture);
		PreviousTexture = Texture;
	}
#pragma endregion

#pragma region UpSample
	float Radius = QxPostprocessSubsystem->GetBloomSettingAsset()->BloomRadius;
	if (CVarQxBloomRadius.GetValueOnRenderThread() != 0)
	{
		Radius = CVarQxBloomRadius.GetValueOnRenderThread();
	}

	// downsamples 的结果拷贝到upsamples中以备后续访问
	UpSampleTextures.Append(DownSampleTextures);

	// Stars at -2 since we need the last buffer
	// as the previous input (-2) and the one just
	// before as the current input (-1).
	// We also go from end to start of array to
	// go from small to big texture (going back up the mips)
	for (int i = PassAmount - 2; i >= 0; --i)
	{
		FIntRect CurrentSize = UpSampleTextures[i].ViewRect;

		const FString PassName  = "UpsampleCombine_"
								+ FString::FromInt( i )
								+ "_"
								+ FString::FromInt( CurrentSize.Width() )
								+ "x"
								+ FString::FromInt( CurrentSize.Height() );

		FRDGTextureRef ResultTexture = RenderUpsampleCombine(
			GraphBuilder,
			PassName,
			ViewInfo,
			UpSampleTextures[i], // Current Texture
			UpSampleTextures[i + 1], // Previous Texture
			Radius
			);

		FScreenPassTexture NewTexture(ResultTexture, CurrentSize);
		UpSampleTextures[i] = NewTexture;
	}
#pragma endregion

	// up sample texture 0 是最终升采样得到的最终结果
	return UpSampleTextures[0]; 
}

FScreenPassTexture FQxBloomSceneViewExtension::RenderFlare(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FScreenPassTexture InputTexture)
{
	RDG_EVENT_SCOPE(GraphBuilder, "QxFlarePasses");
	FScreenPassTexture OutputTexture = InputTexture;

	// 准备不同尺寸的view rect给后面用
#pragma region PrepareDiffSizeViewRect
	const FIntRect ViewRect1 = InputTexture.ViewRect;
	const FIntRect ViewRect2 = FIntRect(
		0, 0,
		ViewRect1.Width() / 2, ViewRect1.Height() / 2
		); 
	const FIntRect ViewRect4 = FIntRect(
		0, 0,
		ViewRect1.Width() / 4, ViewRect1.Height() / 4
		);
#pragma endregion

	UQxBloomFlareAsset* QxPostSettings =  QxPostprocessSubsystem->GetBloomSettingAsset();
	
	FScreenPassTexture IntermediateResult = InputTexture;
	// 添加色差的pass
	if (QxPostSettings->bEnableQxChroma)
	{
		const FString PassName("QxFlareChromaGhost");

		// create chroma target texture
		FRDGTextureDesc TexDesc = InputTexture.Texture->Desc;
		TexDesc.Reset();
		TexDesc.Extent = ViewRect2.Size();
		TexDesc.Format = PF_FloatRGB;
		TexDesc.ClearValue = FClearValueBinding(FLinearColor::Black);

		FRDGTextureRef ChromaTarget = GraphBuilder.CreateTexture(TexDesc, *PassName);

		FQxFlareChromaPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxFlareChromaPS::FParameters>();
		PassParams->Pass.InputTexture = InputTexture.Texture;
		PassParams->Pass.InputTextureSampler = BilinearBorderSampler;
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(ChromaTarget, ERenderTargetLoadAction::ENoAction);
		PassParams->ChromaShift = QxPostprocessSubsystem->GetBloomSettingAsset()->GhostChromaShift;
		
		TShaderMapRef<FQxFlareChromaPS> PixelShader(ViewInfo.ShaderMap);
		// FPixelShaderUtils::AddFullscreenPass(
		// 	GraphBuilder,
		// 	ViewInfo.ShaderMap,
		// 	RDG_EVENT_NAME("QxFlareChromaGhost"),
		// 	PixelShader,
		// 	PassParams,
		// 	ViewRect2
		// 	);
		QxRenderPPUtils::QxDrawScreenPass(
			GraphBuilder,
			TEXT("QxFlareChromaGhost"),
			PixelShader,
			PassParams,
			ViewRect2
			);
		IntermediateResult.Texture = ChromaTarget;
		IntermediateResult.ViewRect = ViewRect2;
	}

	// Flare Ghost Pass
	if (QxPostSettings->bEnableGhost)
	{
		const FString PassName("QxFlareGhosts");

		// 创建一个ghost target texture
		FRDGTextureDesc TexDesc = InputTexture.Texture->Desc;
		TexDesc.Reset();
		TexDesc.Extent = ViewRect2.Size();
		TexDesc.Format = PF_FloatRGB;
		TexDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef GhostTargetTexture = GraphBuilder.CreateTexture(TexDesc, TEXT("QxGhostTargetTexture"));

		TShaderMapRef<FQxFlareGhostPS> PixelShader(ViewInfo.ShaderMap);

		FQxFlareGhostPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxFlareGhostPS::FParameters>();
		PassParams->Pass.InputTexture = IntermediateResult.Texture;
		PassParams->Pass.InputTextureSampler = BilinearBorderSampler;
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(GhostTargetTexture, ERenderTargetLoadAction::ENoAction);
		PassParams->Intensity = QxPostSettings->GhostIntensity;
		// 设置ghost color 数组和scale 数组,从asset 获取
		{
			PassParams->GhostColors[0] = QxPostSettings->Ghost1.Color;
			PassParams->GhostColors[1] = QxPostSettings->Ghost2.Color;
			PassParams->GhostColors[2] = QxPostSettings->Ghost3.Color;
			PassParams->GhostColors[3] = QxPostSettings->Ghost4.Color;
			PassParams->GhostColors[4] = QxPostSettings->Ghost5.Color;
			PassParams->GhostColors[5] = QxPostSettings->Ghost6.Color;
			PassParams->GhostColors[6] = QxPostSettings->Ghost7.Color;
			PassParams->GhostColors[7] = QxPostSettings->Ghost8.Color;

			PassParams->GhostScales[0] = QxPostSettings->Ghost1.Scale;
			PassParams->GhostScales[1] = QxPostSettings->Ghost2.Scale;
			PassParams->GhostScales[2] = QxPostSettings->Ghost3.Scale;
			PassParams->GhostScales[3] = QxPostSettings->Ghost4.Scale;
			PassParams->GhostScales[4] = QxPostSettings->Ghost5.Scale;
			PassParams->GhostScales[5] = QxPostSettings->Ghost6.Scale;
			PassParams->GhostScales[6] = QxPostSettings->Ghost7.Scale;
			PassParams->GhostScales[7] = QxPostSettings->Ghost8.Scale;
		}

		
		// FPixelShaderUtils::AddFullscreenPass(
		// 	GraphBuilder,
		// 	ViewInfo.ShaderMap,
		// 	RDG_EVENT_NAME("QxFlareGhosts"),
		// 	PixelShader,
		// 	PassParams,
		// 	ViewRect2
		// 	);
		QxRenderPPUtils::QxDrawScreenPass(
			GraphBuilder,
			TEXT("QxFlareGhosts"),
			PixelShader,
			PassParams,
			ViewRect2
			);
		IntermediateResult.Texture = GhostTargetTexture;
		IntermediateResult.ViewRect = ViewRect2;
	}

	// Halo Pass
	if (QxPostSettings->bEnableHalo)
	{
		const FString PassName("QxHaloPass");

		TShaderMapRef<FQxHaloPS> PixelShader(ViewInfo.ShaderMap);

		// 如果前面的pass没有给intermediate 创建临时texture，这里需要创建一个，同一个pass不能同时使用一个texture 作为srv/rtv
		if (IntermediateResult.Texture == InputTexture.Texture)
		{
			FRDGTextureDesc TexDesc = InputTexture.Texture->Desc;
			TexDesc.Reset();
			TexDesc.Extent = ViewRect2.Size();
			TexDesc.Format = PF_FloatRGB;
			TexDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
			FRDGTextureRef HaloTargetTexture = GraphBuilder.CreateTexture(TexDesc, TEXT("QxHaloTargetTexture"));
			IntermediateResult.Texture = HaloTargetTexture;
			IntermediateResult.ViewRect = ViewRect2;
		}
		
		FQxHaloPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxHaloPS::FParameters>();
		PassParams->Pass.InputTexture = InputTexture.Texture;
		PassParams->Pass.InputTextureSampler = BilinearBorderSampler;
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(IntermediateResult.Texture,
			ERenderTargetLoadAction::ENoAction);
		PassParams->Compression = QxPostSettings->HaloCompression;
		PassParams->Intensity = QxPostSettings->HaloIntensity;
		PassParams->Mask = QxPostSettings->HaloMask;
		PassParams->Width = QxPostSettings->HaloWidth;
		PassParams->ChromaShift = QxPostSettings->GhostChromaShift;
		
		FRHIBlendState* AdditiveState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();
		// FPixelShaderUtils::AddFullscreenPass(
		// 	GraphBuilder,
		// 	ViewInfo.ShaderMap,
		// 	RDG_EVENT_NAME("QxHaloPass"),
		// 	PixelShader,
		// 	PassParams,
		// 	ViewRect2,
		// 	AdditiveState
		// 	);
		QxRenderPPUtils::QxDrawScreenPass(
			GraphBuilder,
			TEXT("QxHaloPass"),
			PixelShader,
			PassParams,
			ViewRect2,
			AdditiveState
			);
	}


	// Blur pass
	{
		const int32 BlurSteps  = 1;
		IntermediateResult = QxRenderPPUtils::RenderKawaseBlur(
			GraphBuilder,
			ViewInfo,
			IntermediateResult,
			BlurSteps
			);
	}
	
	OutputTexture = IntermediateResult;
	return OutputTexture;
}


FScreenPassTexture FQxBloomSceneViewExtension::RenderFlare(FRDGBuilder& GraphBuilder,
                                                           const FViewInfo& ViewInfo,
                                                           const FPostProcessMaterialInputs& PostProcessMaterialInput,
                                                           FScreenPassTexture InputTexture)
{
	if (!QxPostprocessSubsystem)
	{
		return FScreenPassTexture();
	}
	// check(InputTexture)
	return FScreenPassTexture();
}

FScreenPassTexture FQxBloomSceneViewExtension::RenderGlare(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FScreenPassTexture& InputTexture)
{
	RDG_EVENT_SCOPE(GraphBuilder, "QxGlarePass");
	FScreenPassTexture OutputTexture = InputTexture;

	UQxBloomFlareAsset* QxPostSettings = QxPostprocessSubsystem->GetBloomSettingAsset();
	FIntRect Viewport4 = FIntRect(
		0, 0,
		InputTexture.ViewRect.Width() / 4,
		InputTexture.ViewRect.Height() / 4
		);

	// 创建target glare texture
	FRDGTextureDesc TexDesc = InputTexture.Texture->Desc;
	TexDesc.Reset();
	TexDesc.Extent = Viewport4.Size();
	TexDesc.Format = PF_FloatRGB;
	TexDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
	FRDGTextureRef GlareTexture = GraphBuilder.CreateTexture(TexDesc, TEXT("GlareTargetTexture"));

	FVector4 PixelSize = FVector4(0, 0, 0, 0);
	PixelSize.X = 1.f / float(Viewport4.Width());
	PixelSize.Y = 1.f / float(Viewport4.Height());
	PixelSize.Z = PixelSize.X;
	PixelSize.W = PixelSize.Y * -1.f;

	FVector2D BufferSize = FVector2D(TexDesc.Extent);
	
	if (QxPostSettings->GlareIntensity > SMALL_NUMBER)
	{
		// 这里计算 需要绘制的 point 数量
		// 由于我们需要 2x2对应于1个point ，所以分辨率/2
		FIntPoint TileCount = Viewport4.Size();
		TileCount.X = TileCount.X / 2;
		TileCount.Y = TileCount.Y / 2;
		const int32 Amount = TileCount.X * TileCount.Y;

		// 计算宽高比以 调整 quad的缩放， 假设宽大于高
		FVector2D BufferRatio = FVector2D(
			float(Viewport4.Height()) / float(Viewport4.Width()),
			1.f
			);
		
#pragma region SetupShaderAndParams
		FQxBloomFlarePassParameters* PassParameters = GraphBuilder.AllocParameters<FQxBloomFlarePassParameters>();
		PassParameters->InputTexture = InputTexture.Texture;
		PassParameters->RenderTargets[0] = FRenderTargetBinding(GlareTexture, ERenderTargetLoadAction::ENoAction);
		PassParameters->InputTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Border, AM_Border, AM_Border>::GetRHI();

		// VS Params
		FQxGlareVS::FParameters VSParams;
		VSParams.Pass = *PassParameters;
		VSParams.PixelSize = PixelSize;
		VSParams.TileCount = TileCount;
		VSParams.BufferSize = BufferSize;

		// GS Params
		FQxGlareGS::FParameters GSParams;
		GSParams.BufferSize = BufferSize;
		GSParams.BufferRatio = BufferRatio;
		GSParams.PixelSize = PixelSize;
		GSParams.GlareIntensity = QxPostSettings->GlareIntensity;
		GSParams.GlareTint = QxPostSettings->GlareTint;
		GSParams.GlareScales[0] = QxPostSettings->GlareScale.X;
		GSParams.GlareScales[1] = QxPostSettings->GlareScale.Y;
		GSParams.GlareScales[2] = QxPostSettings->GlareScale.Z;
		GSParams.GlareDivider = FMath::Max(QxPostSettings->GlareDivider, 0.01f);

		// PS Params
		FQxGlarePS::FParameters PSParams;
		PSParams.GlareTexture = GWhiteTexture->TextureRHI;
		PSParams.GlareTextureSampler = BilinearClampSampler;
		if (nullptr != QxPostSettings->GlareLineMask)
		{
			const FTextureRHIRef TextureRHI = QxPostSettings->GlareLineMask->Resource->TextureRHI;
			PSParams.GlareTexture = TextureRHI;
		}

		TShaderMapRef<FQxGlareVS> VertexShader(ViewInfo.ShaderMap);
		TShaderMapRef<FQxGlarePS> PixelShader(ViewInfo.ShaderMap);
		TShaderMapRef<FQxGlareGS> GeometryShader(ViewInfo.ShaderMap);
#pragma endregion

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("QxGlarePass"),
			PassParameters,
			ERDGPassFlags::Raster,
			[Viewport4, VSParams, GSParams, PSParams,
				VertexShader, GeometryShader, PixelShader, Amount
				](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.SetViewport(Viewport4.Min.X, Viewport4.Min.Y, 0.f,
					Viewport4.Max.X, Viewport4.Max.Y, 1.f);

				FGraphicsPipelineStateInitializer GraphicPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicPSOInit);
				GraphicPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();
				GraphicPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicPSOInit.PrimitiveType = PT_PointList;
				// 这个draw call中我们不需要input 完全有shader生成,所以用empty layout
				GraphicPSOInit.BoundShaderState.VertexDeclarationRHI = GEmptyVertexDeclaration.VertexDeclarationRHI;
				GraphicPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicPSOInit.BoundShaderState.GeometryShaderRHI = GeometryShader.GetGeometryShader();
				GraphicPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicPSOInit);

				SetShaderParameters(RHICmdList, VertexShader,
					VertexShader.GetVertexShader(), VSParams);
				SetShaderParameters(RHICmdList, GeometryShader,
					GeometryShader.GetGeometryShader(), GSParams);
				SetShaderParameters(RHICmdList, PixelShader,
					PixelShader.GetPixelShader(), PSParams);

				RHICmdList.SetStreamSource(0, nullptr, 0);
				RHICmdList.DrawPrimitive(0, 1, Amount);
			}
			);
	}
	
	return OutputTexture;
}