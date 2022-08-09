// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPostprocessSubsystem.h"

#include "QxLensFlareAsset.h"

TAutoConsoleVariable<int32> CVarLensFlareRenderBloom(
	TEXT("r.QxLensFlare.RenderBloom"),
	1,
	TEXT(" 0: Don't mix Bloom into lens-flare\n")
	TEXT(" 1: Mix the Bloom into the lens-flare"),
	ECVF_RenderThreadSafe
	);

TAutoConsoleVariable<int32> CVarLensFlareRenderFlarePass(
	TEXT("r.QxLensFlare.RenderFlare"),
	1,
	TEXT(" 0: Don't render flare pass\n")
	TEXT(" 1: render flare pass"),
	ECVF_RenderThreadSafe
	);

TAutoConsoleVariable<int32> CVarLensFlareRenderGlarePass(
	TEXT("r.QxLensFlare.RenderGlare"),
	1,
	TEXT(" 0: Don't render glare pass\n")
	TEXT(" 1: render glare pass"),
	ECVF_RenderThreadSafe
	);
DECLARE_GPU_STAT(QxLensFlare)

// 所有pass都要用到的rdg buffer input
BEGIN_SHADER_PARAMETER_STRUCT(FQxLensFlarePassParameters,)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

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

IMPLEMENT_GLOBAL_SHADER(FQxScreenPassVS, "/QxPPShaders/ScreenPass.usf", "QxScreenPassVS", SF_Vertex);



// 这里声明匿名空间是防止和global 空间冲突
namespace 
{
#pragma region UtilFunctions
	// 主要用来计算 subregion size
	FVector2D GetInputViewortSize(const FIntRect& Input, const FIntPoint& Extent)
	{
		// Based on
		// GetScreenPassTextureViewportParameters()
		// Engine/Source/Runtime/Renderer/Private/ScreenPass.cpp

		FVector2D ExtentInverse = FVector2D(1.f/ Extent.X, 1.f / Extent.Y);

		FVector2D RectMin = FVector2D(Input.Min);
		FVector2D RectMax = FVector2D(Input.Max);

		FVector2D Min = RectMin * ExtentInverse;
		FVector2D Max = RectMax * ExtentInverse;
		return (Max - Min);
	}

// this function draw a shader into Rendergraph texture
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
#pragma endregion

	
	// TODO SHADER SCREENPASS

	// TODO SHADER RESCALE
	#if WITH_EDITOR
	// Rescale shader
	class FlensFlareRescalePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FlensFlareRescalePS);
		SHADER_USE_PARAMETER_STRUCT(FlensFlareRescalePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputViewportSize)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters&)
		{
			return true;
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FlensFlareRescalePS, "/QxPPShaders/Rescale.usf", "RescalePS", SF_Pixel);
#endif

	// TODO SHADER DOWNSAMPLE
	class FDownsamplePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FDownsamplePS);
		SHADER_USE_PARAMETER_STRUCT(FDownsamplePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputSize)
			SHADER_PARAMETER(float, ThresholdLevel)
			SHADER_PARAMETER(float, ThresholdRange)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FDownsamplePS, "/QxPPShaders/DownsampleThreshold.usf", "DownsampleThresholdPS", SF_Pixel);

#pragma region BlurShaders
	// TODO SHADER KAWASE
	// Blur shader use Kawase method
	class FKawaseBlurDownPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FKawaseBlurDownPS);
		SHADER_USE_PARAMETER_STRUCT(FKawaseBlurDownPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, BufferSize)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return  IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};

	class FKawaseBlurUpPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FKawaseBlurUpPS);
		SHADER_USE_PARAMETER_STRUCT(FKawaseBlurUpPS, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, BufferSize)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FKawaseBlurDownPS, "/QxPPShaders/DualKawaseBlur.usf", "KawaseBlurDownsamplerPS", SF_Pixel);
	IMPLEMENT_GLOBAL_SHADER(FKawaseBlurUpPS,   "/QxPPShaders/DualKawaseBlur.usf", "KawaseBlurUpsamplePS", SF_Pixel);

#pragma endregion
	
	// TODO SHADER CHROMA
	class FLensFlareChromaPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlareChromaPS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlareChromaPS, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(float, ChromaShift)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FLensFlareChromaPS, "/QxPPShaders/Chroma.usf", "ChromaPS", SF_Pixel);
	
	// TODO SHADER GHOSTS
	class FLensFlareGhostPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlareGhostPS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlareGhostPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SampleState, InputTextureSampler)
			SHADER_PARAMETER_ARRAY(FVector4, GhostColors, [8])
			SHADER_PARAMETER_ARRAY(float, GhostScales, [8])
			SHADER_PARAMETER(float, Instensity)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		// static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);	
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FLensFlareGhostPS, "/QxPPShaders/Ghosts.usf", "GhostPS", SF_Pixel);
	
	// TODO SHADER HALO
	class FLensFlaresHaloPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlaresHaloPS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlaresHaloPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
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
	IMPLEMENT_GLOBAL_SHADER(FLensFlaresHaloPS, "/QxPPShaders/Halo.usf", "HaloPS", SF_Pixel);

	// TODO SHADER GLARE
#pragma region GlareShaders
	class FLensFlareGlareVS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlareGlareVS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlareGlareVS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FIntPoint, TileCount)
			SHADER_PARAMETER(FVector4, PixelSize)
			SHADER_PARAMETER(FVector2D, BufferSize)
		END_SHADER_PARAMETER_STRUCT()

	};
	IMPLEMENT_GLOBAL_SHADER(FLensFlareGlareVS, "/QxPPShaders/Glare.usf", "GlareVS", SF_Vertex);

	class FLensFlareGlareGS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlareGlareGS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlareGlareGS, FGlobalShader);

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
	IMPLEMENT_GLOBAL_SHADER(FLensFlareGlareGS, "/QxPPShaders/Glare.usf", "GlareGS", SF_Geometry);

	class FLensFlareGlarePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlareGlarePS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlareGlarePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_SAMPLER(SamplerState, GlareTextureSampler)
			SHADER_PARAMETER_TEXTURE(Texture2D, GlareTexture)
		END_SHADER_PARAMETER_STRUCT()
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);	
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FLensFlareGlarePS, "/QxPPShaders/Glare.usf", "GlarePS", SF_Pixel);
#pragma endregion

	
	// TODO SHADER MIX
	class FLensFlareBloomMixPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FLensFlareBloomMixPS);
		SHADER_USE_PARAMETER_STRUCT(FLensFlareBloomMixPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BloomTexture)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GlareTexture)
			SHADER_PARAMETER_TEXTURE(Texture2D, GradientTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, GradientTextureSampler)
			SHADER_PARAMETER(FVector4, Tint)
			SHADER_PARAMETER(FVector2D, InputViewportSize)
			SHADER_PARAMETER(FVector2D, BufferSize)
			SHADER_PARAMETER(FVector2D, PixelSize)
			SHADER_PARAMETER(FIntVector, MixPass)
			SHADER_PARAMETER(float, Intensity)
		END_SHADER_PARAMETER_STRUCT()
		
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};

	IMPLEMENT_GLOBAL_SHADER(FLensFlareBloomMixPS, "/QxPPShaders/Mix.usf", "MixPS", SF_Pixel);
}

void UQxPostprocessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 注意： 按照当前的代码没有考虑到线程安全，如果要做按文档中推荐应该是将渲染代码移到一个子类中用
	// 线程安全的TSharePtr持有这个对象
	Super::Initialize(Collection);

	// Delegate setup
	FPP_LensFlares::FDelegate Delegate = FPP_LensFlares::FDelegate::CreateLambda(
		[=](FRDGBuilder& GraphBuilder, const FViewInfo& View,
			const FLensFlareInputs& Inputs,
			FLensFlareOutputData& Outputs)
		{
			RenderLensFlare(GraphBuilder, View, Inputs, Outputs);
		}
		);

	ENQUEUE_RENDER_COMMAND(BindRenderThreadDelegates)(
		[Delegate](FRHICommandListImmediate& RHICmdList)
		{
			PP_LensFlares.Add(Delegate);
		}
		);

	// data asset loading
	// #TODO 这里先朝下，一会改
	FString Path = "PostProcessLensFlareAsset'/CustomPostProcess/DefaultLensFlare.DefaultLensFlare'";

	PostprocessAsset = LoadObject<UQxLensFlareAsset>(nullptr, *Path);
	check(PostprocessAsset);
}

void UQxPostprocessSubsystem::Deinitialize()
{
	ClearBlendState = nullptr;
	AdditiveBlendState = nullptr;
	BilinearClampSampler = nullptr;
	BilinearBorderSampler = nullptr;
	BilinearRepeatSampler = nullptr;
	NearestRepeatSampler = nullptr;
	Super::Deinitialize();
}

void UQxPostprocessSubsystem::RenderLensFlare(FRDGBuilder& GraphBuilder, const FViewInfo& View,
	const FLensFlareInputs& Inputs, FLensFlareOutputData& Outputs)
{
	check(IsInRenderingThread());
	check(Inputs.Bloom.IsValid());
	check(Inputs.HalfScreenColor.IsValid());

	if (PostprocessAsset == nullptr)
	{
		return;
	}
	
	RDG_GPU_STAT_SCOPE(GraphBuilder, QxLensFlare);
	RDG_EVENT_SCOPE(GraphBuilder, "QxLensFlare");

#pragma region SetupResourceVariables
	const FScreenPassTextureViewport BloomViewport(Inputs.Bloom);
	const FVector2D BloomInputViewportSize = GetInputViewortSize(BloomViewport.Rect, BloomViewport.Extent);

	const FScreenPassTextureViewport SceneColorViewport(Inputs.HalfScreenColor);
	const FVector2D SceneColorViewportSize = GetInputViewortSize(SceneColorViewport.Rect, SceneColorViewport.Extent);

	// input
	FRDGTextureRef InputTexture = Inputs.HalfScreenColor.Texture;
	FIntRect InputRect = SceneColorViewport.Rect;

	// Ouputs
	FRDGTextureRef OutputTexture = Inputs.HalfScreenColor.Texture;
	FIntRect OutputRect = SceneColorViewport.Rect;

	// blend states
	if (nullptr == ClearBlendState)
	{
		// Blend modes from:
		// '/Engine/Source/Runtime/RenderCore/Private/ClearQuad.cpp'
		// '/Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessMaterial.cpp'
		ClearBlendState = TStaticBlendState<>::GetRHI();
		AdditiveBlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();
		
		BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		BilinearBorderSampler = TStaticSamplerState<SF_Bilinear, AM_Border, AM_Border, AM_Border>::GetRHI();
		BilinearRepeatSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();

		NearestRepeatSampler = TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
	}

	// TODO RESCALE
#pragma region Rescale
#if WITH_EDITOR
	if ((SceneColorViewport.Rect.Width() != SceneColorViewport.Extent.X )
		|| (SceneColorViewport.Rect.Height() != SceneColorViewport.Extent.Y))
	{
		const FString PassName("LensFlareRescale");

		// Build target buffer;
		FRDGTextureDesc Desc = Inputs.HalfScreenColor.Texture->Desc;
		Desc.Reset();
		Desc.Extent = SceneColorViewport.Rect.Size();
		Desc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef RescaleTexture = GraphBuilder.CreateTexture(Desc, *PassName);

		// Setup shaders
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FlensFlareRescalePS> PixelShader(View.ShaderMap);

		// Setup Shader parameters
		FlensFlareRescalePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FlensFlareRescalePS::FParameters>();

		PassParameters->Pass.InputTexture = Inputs.HalfScreenColor.Texture;
		PassParameters->Pass.RenderTargets[0] = FRenderTargetBinding(RescaleTexture, ERenderTargetLoadAction::ENoAction);
		PassParameters->InputTextureSampler = BilinearClampSampler;
		PassParameters->InputViewportSize = SceneColorViewportSize;

		// Render Shader Into buffer;
		DrawShaderpass(
			GraphBuilder,
			PassName,
			PassParameters,
			VertexShader,
			PixelShader,
			ClearBlendState,
			SceneColorViewport.Rect
			);

		// Assign result before end of scope
		InputTexture = RescaleTexture;
	}
#endif
#pragma endregion

	// Render passes
	FRDGTextureRef ThresholdTexture = nullptr;
	FRDGTextureRef FlareTexture = nullptr;
	FRDGTextureRef GlareTexture = nullptr;

	ThresholdTexture = RenderThreshold(
		GraphBuilder,
		InputTexture,
		InputRect,
		View
		);

	if (CVarLensFlareRenderFlarePass.GetValueOnRenderThread())
	{
		FlareTexture = RenderFlare(
			GraphBuilder,
			ThresholdTexture,
			InputRect,
			View
			);
	}

	if (CVarLensFlareRenderGlarePass.GetValueOnRenderThread())
	{
		GlareTexture = RenderGlare(
			GraphBuilder,
			ThresholdTexture,
			InputRect,
			View
			);
	}

	// TODO MIX
	{
		const FString PassName("LensFlareMix");

		FIntRect MixViewport = FIntRect(
			0, 0,
			View.ViewRect.Width() / 2,
			View.ViewRect.Height() / 2
			);

		FVector2D BufferSize = FVector2D(MixViewport.Width(), MixViewport.Height());

		// Create buffer
		FRDGTextureDesc Decription = Inputs.Bloom.Texture->Desc;
		Decription.Reset();
		Decription.Extent = MixViewport.Size();
		Decription.Format = PF_FloatRGBA;
		Decription.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef MixTexture = GraphBuilder.CreateTexture(Decription, *PassName);

#pragma region ShaderParamSetup

		// shader parameters
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FLensFlareBloomMixPS> PixelShader(View.ShaderMap);

		FLensFlareBloomMixPS::FParameters* PassParams = GraphBuilder.AllocParameters<FLensFlareBloomMixPS::FParameters>();
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(MixTexture, ERenderTargetLoadAction::ENoAction);
		PassParams->InputTextureSampler = BilinearClampSampler;
		PassParams->GradientTexture = GWhiteTexture->TextureRHI;
		PassParams->GradientTextureSampler = BilinearClampSampler;
		PassParams->BufferSize = BufferSize;
		PassParams->PixelSize = FVector2D(1.f, 1.f) / BufferSize;
		PassParams->InputViewportSize = BloomInputViewportSize;
		PassParams->Tint = FVector4(PostprocessAsset->Tint);
		PassParams->Intensity = PostprocessAsset->Intensity;

		if (PostprocessAsset->Gradient != nullptr)
		{
			const FTextureRHIRef TextureRHI = PostprocessAsset->Gradient->Resource->TextureRHI;
			PassParams->GradientTexture = TextureRHI;
		}

		// Plugin in buffers
		const int32 MixBloomPass = CVarLensFlareRenderBloom.GetValueOnRenderThread();

		PassParams->MixPass = FIntVector(
			(Inputs.bCompositeWithBloom && MixBloomPass),
			(FlareTexture != nullptr),
			(GlareTexture != nullptr)
			);

		if (Inputs.bCompositeWithBloom && MixBloomPass)
		{
			PassParams->BloomTexture = Inputs.Bloom.Texture;
		}
		else
		{
			PassParams->BloomTexture = InputTexture;
		}

		if (FlareTexture != nullptr)
		{
			PassParams->Pass.InputTexture = FlareTexture;
		}
		else
		{
			PassParams->Pass.InputTexture = InputTexture;
		}

		if (GlareTexture != nullptr)
		{
			PassParams->GlareTexture = GlareTexture;
		}
		else
		{
			PassParams->GlareTexture = InputTexture;
		}
#pragma endregion

		// Render
		DrawShaderpass(
			GraphBuilder,
			PassName,
			PassParams,
			VertexShader,
			PixelShader,
			ClearBlendState,
			MixViewport
			);
	}

	
	// Final out
	Outputs.Texture = OutputTexture;
	Outputs.Rect = OutputRect;
#pragma endregion
}

FRDGTextureRef UQxPostprocessSubsystem::RenderThreshold(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
	FIntRect& InputRect, const FViewInfo& View)
{
	RDG_EVENT_SCOPE(GraphBuilder, "Threshold pass");

	FRDGTextureRef OutputTexture = nullptr;
	FIntRect Viewport = View.ViewRect;
	FIntRect Viewport2 = FIntRect(0, 0,
		View.ViewRect.Width() / 2,
		View.ViewRect.Height() / 2);
	FIntRect Viewport4 = FIntRect(0, 0,
		View.ViewRect.Width() / 4,
		View.ViewRect.Height() / 4);
	// Threshold
	{
		const FString PassName("LensFlareDownsample");

		// Build Texture
		FRDGTextureDesc Description = InputTexture->Desc;
		Description.Reset();
		Description.Extent = Viewport4.Size();
		Description.Format = PF_FloatRGB;
		Description.ClearValue = FClearValueBinding(FLinearColor::Black);
		FRDGTextureRef Texture = GraphBuilder.CreateTexture(Description, *PassName);

		// Render shader
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FDownsamplePS> PixelShader(View.ShaderMap);

		FDownsamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FDownsamplePS::FParameters>();
		PassParameters->Pass.InputTexture = InputTexture;
		PassParameters->Pass.RenderTargets[0] = FRenderTargetBinding(Texture, ERenderTargetLoadAction::ENoAction);
		PassParameters->InputSize = FVector2D(Viewport2.Size());
		PassParameters->ThresholdLevel = PostprocessAsset->ThresholdLevel;
		PassParameters->ThresholdRange = PostprocessAsset->ThresholdRange;

		DrawShaderpass(
			GraphBuilder,
			PassName,
			PassParameters,
			VertexShader,
			PixelShader,
			ClearBlendState,
			Viewport4
			);
		OutputTexture = Texture;
	}
	
	
	// Threshold blur
	{
		OutputTexture = RenderBlur(
			GraphBuilder,
			OutputTexture,
			View,
			Viewport2,
			1
			);
	}
	return OutputTexture;
}

FRDGTextureRef UQxPostprocessSubsystem::RenderFlare(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
                                                    FIntRect& InputRect, const FViewInfo& View)
{
	RDG_EVENT_SCOPE(GraphBuilder, "QxFlarePass");
#pragma region SetupVaribles
	FRDGTextureRef OutputTexture = nullptr;

	FIntRect Viewport = View.ViewRect;
	FIntRect Viewport2 = FIntRect(
		0, 0,
		View.ViewRect.Width() / 2,
		View.ViewRect.Height() / 2
		);
	FIntRect Viewport4 = FIntRect(
		0, 0,
		View.ViewRect.Width() / 4,
		View.ViewRect.Height() / 4
		);

#pragma endregion

#pragma region Flare Chroma
	FRDGTextureRef ChromaTexture = nullptr;

	{
		const FString PassName("LensFlareChromaGhost");

		// build buffer
		FRDGTextureDesc Description = InputTexture->Desc;
		Description.Reset();
		Description.Extent = Viewport2.Size();
		Description.Format = PF_FloatRGB;
		Description.ClearValue = FClearValueBinding(FLinearColor::Black);
		ChromaTexture = GraphBuilder.CreateTexture(Description, *PassName);

		// setup shader parameters
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FLensFlareChromaPS> PixelShader(View.ShaderMap);

		FLensFlareChromaPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FLensFlareChromaPS::FParameters>();
		PassParameters->Pass.InputTexture = InputTexture;
		PassParameters->Pass.RenderTargets[0] = FRenderTargetBinding(ChromaTexture, ERenderTargetLoadAction::ENoAction);
		PassParameters->InputTextureSampler = BilinearBorderSampler;
		PassParameters->ChromaShift = PostprocessAsset->GhostChromaShift;
		
		// Render
		DrawShaderpass(
			GraphBuilder,
			PassName,
			PassParameters,
			VertexShader,
			PixelShader,
			ClearBlendState,
			Viewport2
			);
	}

#pragma endregion

	// Flare Ghosts
	{
		const FString PassName("LensFlareGhosts");

		// Build Buffer
		FRDGTextureDesc Description = InputTexture->Desc;
		Description.Reset();
		Description.Extent = Viewport2.Size();
		Description.Format = PF_FloatRGB;
		Description.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef Texture = GraphBuilder.CreateTexture(Description, *PassName);

		// setup shader parameter
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FLensFlareGhostPS> PixelShader(View.ShaderMap);

		FLensFlareGhostPS::FParameters* ShaderParams = GraphBuilder.AllocParameters<FLensFlareGhostPS::FParameters>();
		ShaderParams->Instensity = PostprocessAsset->GhostIntensity;
		ShaderParams->Pass.InputTexture = ChromaTexture;
		ShaderParams->Pass.RenderTargets[0] = FRenderTargetBinding(Texture, ERenderTargetLoadAction::ENoAction);
		ShaderParams->InputTextureSampler = BilinearBorderSampler;
		// 设置ghost color 数组和scale 数组,从asset 获取
		{
			ShaderParams->GhostColors[0] = PostprocessAsset->Ghost1.Color;
			ShaderParams->GhostColors[1] = PostprocessAsset->Ghost2.Color;
			ShaderParams->GhostColors[2] = PostprocessAsset->Ghost3.Color;
			ShaderParams->GhostColors[3] = PostprocessAsset->Ghost4.Color;
			ShaderParams->GhostColors[4] = PostprocessAsset->Ghost5.Color;
			ShaderParams->GhostColors[5] = PostprocessAsset->Ghost6.Color;
			ShaderParams->GhostColors[6] = PostprocessAsset->Ghost7.Color;
			ShaderParams->GhostColors[7] = PostprocessAsset->Ghost8.Color;

			ShaderParams->GhostScales[0] = PostprocessAsset->Ghost1.Scale;
			ShaderParams->GhostScales[1] = PostprocessAsset->Ghost2.Scale;
			ShaderParams->GhostScales[2] = PostprocessAsset->Ghost3.Scale;
			ShaderParams->GhostScales[3] = PostprocessAsset->Ghost4.Scale;
			ShaderParams->GhostScales[4] = PostprocessAsset->Ghost5.Scale;
			ShaderParams->GhostScales[5] = PostprocessAsset->Ghost6.Scale;
			ShaderParams->GhostScales[6] = PostprocessAsset->Ghost7.Scale;
			ShaderParams->GhostScales[7] = PostprocessAsset->Ghost8.Scale;
		}
		
		// Render Shader
		DrawShaderpass(
			GraphBuilder,
			PassName,
			ShaderParams,
			VertexShader,
			PixelShader,
			ClearBlendState,
			Viewport2
			);
		OutputTexture = Texture;
	}

	// Flare Halo
	{
		const FString PassName("LensFlareHalo");

		TShaderMapRef<FQxScreenPassVS> VertexShder(View.ShaderMap);
		TShaderMapRef<FLensFlaresHaloPS> PixelShader(View.ShaderMap);

		FLensFlaresHaloPS::FParameters* ShaderParams = GraphBuilder.AllocParameters<FLensFlaresHaloPS::FParameters>();
		ShaderParams->Compression = PostprocessAsset->HaloCompression;
		ShaderParams->Intensity = PostprocessAsset->HaloIntensity;
		ShaderParams->Mask = PostprocessAsset->HaloMask;
		ShaderParams->Width = PostprocessAsset->HaloWidth;
		ShaderParams->ChromaShift = PostprocessAsset->HaloChromaShift;
		ShaderParams->InputTextureSampler = BilinearBorderSampler;
		ShaderParams->Pass.InputTexture = InputTexture;
		ShaderParams->Pass.RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::ELoad);

		DrawShaderpass(
			GraphBuilder,
			PassName,
			ShaderParams,
			VertexShder,
			PixelShader,
			AdditiveBlendState,
			Viewport2
			);
	}

	{
		OutputTexture = RenderBlur(
			GraphBuilder,
			OutputTexture,
			View,
			Viewport2,
			1
			);
	}

	return OutputTexture;
}

FRDGTextureRef UQxPostprocessSubsystem::RenderGlare(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
                                                    FIntRect& InputRect, const FViewInfo& View)
{
	RDG_EVENT_SCOPE(GraphBuilder, "QxGlarePass");

#pragma region PrepareVariables
	FRDGTextureRef OutputTexture = nullptr;

	FIntRect Viewport4 = FIntRect(
		0, 0,
		View.ViewRect.Width() / 4,
		View.ViewRect.Height() / 4
		);
#pragma endregion
	// 不接近0 才渲染glare
	if (PostprocessAsset->GlareIntensity > SMALL_NUMBER)
	{
		const FString PassName("LensFlareGlare");

		// 这里计算 需要绘制的 point 数量
		// 由于我们需要 2x2对应于1个point ，所以分辨率/2
		FIntPoint TileCount = Viewport4.Size();
		TileCount.X = TileCount.X / 2;
		TileCount.Y = TileCount.Y / 2;
		int32 Amount = TileCount.X * TileCount.Y;

		// 计算宽高比以 调整 quad的缩放， 假设宽大于高
		FVector2D BufferRatio = FVector2D(
			float(Viewport4.Height()) / float(Viewport4.Width()),
			1.f
			);

		// build the buffer
		FRDGTextureDesc Description = InputTexture->Desc;
		Description.Reset();
		Description.Extent = Viewport4.Size();
		Description.Format = PF_FloatRGB;
		Description.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef GlareTexture = GraphBuilder.CreateTexture(Description, *PassName);

		// 设置shader 需要的其他参数
		FVector4 PixelSize = FVector4(0.f, 0.f, 0.f, 0.f);
		PixelSize.X = 1.0f / float(Viewport4.Width());
		PixelSize.Y = 1.0f / float(Viewport4.Height());
		PixelSize.Z = PixelSize.X;
		PixelSize.W = PixelSize.Y * -1.0f;

		FVector2D BufferSize = FVector2D(Description.Extent);

#pragma region SetupShader
		// Setup Shader
		FQxLensFlarePassParameters* PassParameters = GraphBuilder.AllocParameters<FQxLensFlarePassParameters>();
		PassParameters->InputTexture = InputTexture;
		PassParameters->RenderTargets[0] = FRenderTargetBinding(GlareTexture, ERenderTargetLoadAction::EClear);

		// Vertex Shader
		FLensFlareGlareVS::FParameters VertexParameters;
		VertexParameters.Pass = *PassParameters;
		VertexParameters.InputTextureSampler = BilinearBorderSampler;
		VertexParameters.TileCount = TileCount;
		VertexParameters.PixelSize = PixelSize;
		VertexParameters.BufferSize = BufferSize;

		// Geometry shader
		FLensFlareGlareGS::FParameters GeometryParameters;
		GeometryParameters.BufferSize = BufferSize;
		GeometryParameters.BufferRatio = BufferRatio;
		GeometryParameters.PixelSize = PixelSize;
		GeometryParameters.GlareIntensity = PostprocessAsset->GlareIntensity;
		GeometryParameters.GlareTint = FVector4(PostprocessAsset->GlareTint);
		GeometryParameters.GlareScales[0] = PostprocessAsset->GlareScale.X;
		GeometryParameters.GlareScales[1] = PostprocessAsset->GlareScale.Y;
		GeometryParameters.GlareScales[2] = PostprocessAsset->GlareScale.Z;
		GeometryParameters.GlareDivider = FMath::Max(PostprocessAsset->GlareDivider, 0.01f);

		// Pixel Shader
		FLensFlareGlarePS::FParameters PixelParameters;
		PixelParameters.GlareTexture = GWhiteTexture->TextureRHI;
		PixelParameters.GlareTextureSampler = BilinearClampSampler;

		if (nullptr != PostprocessAsset->GlareLineMask)
		{
			const FTextureRHIRef TextureRHI = PostprocessAsset->GlareLineMask->Resource->TextureRHI;
			PixelParameters.GlareTexture = TextureRHI;
		}

		TShaderMapRef<FLensFlareGlareVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FLensFlareGlareGS> GeometryShader(View.ShaderMap);
		TShaderMapRef<FLensFlareGlarePS> PixelShader(View.ShaderMap);
#pragma endregion

#pragma region NAME
		FRHIBlendState* BlendState = this->AdditiveBlendState;

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("%s", *PassName),
			PassParameters,
			ERDGPassFlags::Raster,
			[
				VertexShader,VertexParameters,
				GeometryShader, GeometryParameters,
				PixelShader, PixelParameters,
				BlendState, Viewport4, Amount
				](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.SetViewport(Viewport4.Min.X, Viewport4.Min.Y, 0.0f,
					Viewport4.Max.X, Viewport4.Max.Y, 1.0f);

				FGraphicsPipelineStateInitializer GraphicPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicPSOInit); //#TODO 具体分析这句话的作用
				GraphicPSOInit.BlendState = BlendState;
				GraphicPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicPSOInit.BoundShaderState.VertexDeclarationRHI = GEmptyVertexDeclaration.VertexDeclarationRHI;
				GraphicPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicPSOInit.BoundShaderState.GeometryShaderRHI = GeometryShader.GetGeometryShader();
				GraphicPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicPSOInit.PrimitiveType = PT_PointList;
				SetGraphicsPipelineState(RHICmdList, GraphicPSOInit);

				SetShaderParameters(RHICmdList, VertexShader,
					VertexShader.GetVertexShader(), VertexParameters);
				SetShaderParameters(RHICmdList, GeometryShader,
					GeometryShader.GetGeometryShader(), GeometryParameters);
				SetShaderParameters(RHICmdList, PixelShader,
					PixelShader.GetPixelShader(), PixelParameters);

				RHICmdList.SetStreamSource(0, nullptr, 0);
				RHICmdList.DrawPrimitive(0, 1, Amount);
			}
			);
#pragma endregion

		OutputTexture = GlareTexture;
	}

	return OutputTexture;
}

FRDGTextureRef UQxPostprocessSubsystem::RenderBlur(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
                                                   const FViewInfo& View, const FIntRect& Viewport, int BlurSteps)
{
	// Shader setup
	TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
	TShaderMapRef<FKawaseBlurDownPS> PixelShaderDown(View.ShaderMap);
	TShaderMapRef<FKawaseBlurUpPS> PixelShaderUp(View.ShaderMap);

	// Data setup
	FRDGTextureRef PreviousBuffer = InputTexture;
	const FRDGTextureDesc& InputDesc = InputTexture->Desc;

	const FString PassDownName = TEXT("Down");
	const FString PassUpName = TEXT("Up");
	const int32 ArraySize = BlurSteps * 2;

	// viewport resolutions
	int32 Divider = 2;
	TArray<FIntRect> Viewports;
	for (int32 i = 0; i < ArraySize; ++i)
	{
		FIntRect NewRect = FIntRect(
			0,
			0,
			Viewport.Width() / Divider,
			Viewport.Height() / Divider
			);

		Viewports.Add(NewRect);
		// 这里的判断和blur step的意思是blurstep以下是downsample， 以上是upsample
		if (i < (BlurSteps - 1))
		{
			Divider *= 2;
		}
		else
		{
			Divider -= 2;
		}
	}

	// Render
	for (int32 i = 0; i < ArraySize; ++i)
	{
		FRDGTextureDesc BlurDesc = InputDesc;
		BlurDesc.Reset();
		BlurDesc.Extent = Viewports[i].Size();
		BlurDesc.Format = PF_FloatRGB;
		BlurDesc.NumMips = 1;
		BlurDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);

		FVector2D ViewportResolution = FVector2D(
			Viewports[i].Width(),
			Viewports[i].Height()
			);

		const FString PassName =
			FString("KawaseBlur")
			+ FString::Printf(TEXT("_%i_"), i)
			+ ( (i < BlurSteps) ? PassDownName : PassUpName)
			+ FString::Printf( TEXT("_%ix%i"), Viewports[i].Width(), Viewports[i].Height());

		FRDGTextureRef Buffer = GraphBuilder.CreateTexture(BlurDesc, *PassName);

		// Render shader
		if( i < BlurSteps )
		{
			FKawaseBlurDownPS::FParameters* PassDownParameters = GraphBuilder.AllocParameters<FKawaseBlurDownPS::FParameters>();
			PassDownParameters->Pass.InputTexture       = PreviousBuffer;
			PassDownParameters->Pass.RenderTargets[0]   = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ENoAction);
			PassDownParameters->InputTextureSampler            = BilinearClampSampler;
			PassDownParameters->BufferSize              = ViewportResolution;

			DrawShaderpass(
				GraphBuilder,
				PassName,
				PassDownParameters,
				VertexShader,
				PixelShaderDown,
				ClearBlendState,
				Viewports[i]
			);
		}
		else
		{
			FKawaseBlurUpPS::FParameters* PassUpParameters = GraphBuilder.AllocParameters<FKawaseBlurUpPS::FParameters>();
			PassUpParameters->Pass.InputTexture         = PreviousBuffer;
			PassUpParameters->Pass.RenderTargets[0]     = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ENoAction);
			PassUpParameters->InputTextureSampler              = BilinearClampSampler;
			PassUpParameters->BufferSize                = ViewportResolution;

			DrawShaderpass(
				GraphBuilder,
				PassName,
				PassUpParameters,
				VertexShader,
				PixelShaderUp,
				ClearBlendState,
				Viewports[i]
			);
		}

		PreviousBuffer = Buffer;
	}
	return PreviousBuffer;
}
