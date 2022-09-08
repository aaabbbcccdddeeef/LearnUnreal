// Fill out your copyright notice in the Description page of Project Settings.


#include "QxBloomSubsystem.h"

#include <stdexcept>

#include "QxLensFlareAsset.h"
#include "Interfaces/IPluginManager.h"

#include "RenderGraph.h"
#include "ScreenPass.h"
#include "SystemTextures.h"

//extern  TGlobalResource<FSystemTextures> GSystemTextures;

TAutoConsoleVariable<int32> CVarBloomPassAmount(
	TEXT("r.QxRender.BloomPassAmount"),
	8,
	TEXT("Number of passes to render bloom"),
	ECVF_RenderThreadSafe
	);

TAutoConsoleVariable<float> CVarBloomRadius(
	TEXT("r.QxRender.BloomRadius"),
	0.85,
	TEXT("Size/Scale of the Bloom")
	);
DECLARE_GPU_STAT(PostProcessQx);

namespace
{
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
	
	// Bloom down sample
	class FDownSamplePS_Bloom : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FDownSamplePS_Bloom);
		SHADER_USE_PARAMETER_STRUCT(FDownSamplePS_Bloom, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputSize)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FDownSamplePS_Bloom, "/QxPPShaders/Bloom/DownSample.usf", "DownSamplePS", SF_Pixel);

	// Bloom upsample + combine
	class FUpsampleCombinePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FUpsampleCombinePS);
		SHADER_USE_PARAMETER_STRUCT(FUpsampleCombinePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputSize)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, PreviousTexture)
			SHADER_PARAMETER(float, Radius)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FUpsampleCombinePS, "/QxPPShaders/Bloom/UpsampleCombine.usf", "UpsampleCombinePS", SF_Pixel);

	// TODO SHADER MIX
	class FMixPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FMixPS);
		SHADER_USE_PARAMETER_STRUCT(FMixPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxLensFlarePassParameters, Pass)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, BloomTexture)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GlareTexture)
			SHADER_PARAMETER_TEXTURE(Texture2D, FlareGradientTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, FlareGradientTextureSampler)
			SHADER_PARAMETER(FVector4, FlareTint)
			SHADER_PARAMETER(FVector2D, InputViewportSize)
			SHADER_PARAMETER(FVector2D, BufferSize)
			SHADER_PARAMETER(FVector2D, GlarePixelSize)
			SHADER_PARAMETER(FIntVector, MixPass)
			SHADER_PARAMETER(float, BloomIntensity)
			SHADER_PARAMETER(float, FlareIntensity)
		END_SHADER_PARAMETER_STRUCT()
		
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};

	IMPLEMENT_GLOBAL_SHADER(FMixPS, "/QxPPShaders/Bloom/BloomMix.usf", "MixPS", SF_Pixel);

#if WITH_EDITOR
	// Rescale shader
	class FRescalePS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FRescalePS);
		SHADER_USE_PARAMETER_STRUCT(FRescalePS, FGlobalShader);

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
	IMPLEMENT_GLOBAL_SHADER(FRescalePS, "/QxPPShaders/Rescale.usf", "RescalePS", SF_Pixel);
#endif
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
}


void UQxBloomSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// setup delegate
	FPP_CustomBloomFlare::FDelegate Delegate = FPP_CustomBloomFlare::FDelegate::CreateLambda(
		[=](FRDGBuilder& GraphBuilder, const FViewInfo& View,
			const FScreenPassTexture& SceneColor, FScreenPassTexture& Output)
		{
			Render(GraphBuilder, View, SceneColor, Output);
		}
		);

	ENQUEUE_RENDER_COMMAND(QxBloom)(
		[Delegate](FRHICommandListImmediate& RHICmdList)
		{
			PP_CustomFlare.Add(Delegate);
		}
		);

	// data asset loading
	// #TODO 这里先朝下，一会改
	FString Path = "QxLensFlareAsset'/Game/QxLensFlare/DefaultLensFlare.DefaultLensFlare'";

	PostprocessAsset = LoadObject<UQxLensFlareAsset>(nullptr, *Path);
	check(PostprocessAsset);
}

void UQxBloomSubsystem::Deinitialize()
{
	ClearBlendState = nullptr;
	AdditiveBlendState = nullptr;

	BilinearBorderSampler = nullptr;
	BilinearClampSampler = nullptr;
	BilinearRepeatSampler = nullptr;
	Super::Deinitialize();
}

void UQxBloomSubsystem::InitStates()
{
	if (ClearBlendState)
	{
		return;
	}

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

void UQxBloomSubsystem::Render(FRDGBuilder& GraphBuilder, const FViewInfo& View, const FScreenPassTexture& SceneColor,
	FScreenPassTexture& Output)
{
	check(SceneColor.IsValid());

	if (PostprocessAsset == nullptr)
	{
		return;
	}

	InitStates();

	RDG_GPU_STAT_SCOPE(GraphBuilder, PostProcessQx);
	RDG_EVENT_SCOPE(GraphBuilder, "PostProcessQx");

	int32 PassAmount = CVarBloomPassAmount.GetValueOnRenderThread();

	// Buffers Steup
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
	FScreenPassTexture InputTexture(SceneColor.Texture);

	const FScreenPassTextureViewport SceneColorViewport(SceneColor);
	const FVector2D SceneColorViewportSize = GetInputViewortSize(SceneColorViewport.Rect, SceneColorViewport.Extent);

	#if 0//WITH_EDITOR
	// Editor buffer scale
	// Rescale the Scene Color to fit the whole texture and not use a sub-region.
	// This is to simplify the render pass (shaders) that come after.
	// This part is skipped when built without the editor because
	// it is not needed (unless splitscreen needs it ?).
	if ( SceneColorViewport.Rect.Width() != SceneColorViewport.Extent.X
		|| SceneColorViewport.Rect.Height() != SceneColorViewport.Extent.Y)
	{
		const FString PassName("SceneColorRescale");

		// Build Texture
		FRDGTextureDesc Desc = SceneColor.Texture->Desc;
		Desc.Reset();
		Desc.Extent = SceneColorViewport.Rect.Size();
		Desc.Format =  PF_FloatRGB;
		Desc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef RescaleTexture = GraphBuilder.CreateTexture(Desc, *PassName);

		// Render Shader
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FRescalePS> PixelShader(View.ShaderMap);

		FRescalePS::FParameters* PassParams = GraphBuilder.AllocParameters<FRescalePS::FParameters>();
		PassParams->Pass.InputTexture = SceneColor.Texture;
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(RescaleTexture, ERenderTargetLoadAction::ENoAction);
		PassParams->InputTextureSampler = BilinearClampSampler;
		PassParams->InputViewportSize = SceneColorViewportSize;

		DrawShaderpass(
			GraphBuilder,
			PassName,
			PassParams,
			VertexShader,
			PixelShader,
			ClearBlendState,
			SceneColorViewport.Rect
			);

		InputTexture.Texture = RescaleTexture;
		InputTexture.ViewRect = SceneColorViewport.Rect;
	}
	#endif

	///Render passes
	//Bloom
	{
		BloomTexture = RenderBloom(
			GraphBuilder,
			View,
			InputTexture,
			PassAmount
			);
	}

	// Flare

	// Glare


	/// Composite Bloom, Flare and Glare together
	FRDGTextureRef MixTexture = nullptr;
	FIntRect MixViewport(
		0, 0,
		View.ViewRect.Width() / 2,
		View.ViewRect.Height() / 2
		);
	{
		RDG_EVENT_SCOPE(GraphBuilder, "MixPass");

		const FString PassName("Mix");

		float BloomIntensity = 1.0f;

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
		FRDGTextureDesc Desc = SceneColor.Texture->Desc;
		Desc.Reset();
		Desc.Extent = MixViewport.Size();
		Desc.Format = PF_FloatRGB;
		Desc.ClearValue = FClearValueBinding(FLinearColor::Black);
		MixTexture = GraphBuilder.CreateTexture(Desc, *PassName);

		// Render shader
		TShaderMapRef<FQxScreenPassVS> VertexShader(View.ShaderMap);
		TShaderMapRef<FMixPS> PixelShader(View.ShaderMap);

		FMixPS::FParameters* PassParams = GraphBuilder.AllocParameters<FMixPS::FParameters>();
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(MixTexture, ERenderTargetLoadAction::ENoAction);
		PassParams->InputTextureSampler = BilinearClampSampler;
		PassParams->MixPass = BufferValidity;

		// Bloom
		PassParams->BloomTexture = BlackDummy.Texture;
		PassParams->BloomIntensity = BloomIntensity;

		// Glare
		PassParams->GlareTexture = BlackDummy.Texture;
		PassParams->GlarePixelSize = FVector2D(1.f, 1.f) / BufferSize;

		// Flare
		PassParams->Pass.InputTexture  = BlackDummy.Texture;
		PassParams->FlareIntensity = PostprocessAsset->FlareIntensity;
		PassParams->FlareTint = FVector4(PostprocessAsset->FlareTint);
		PassParams->FlareGradientTexture = GWhiteTexture->TextureRHI;
		PassParams->FlareGradientTextureSampler = BilinearClampSampler;

		if (PostprocessAsset->FlareGradient)
		{
			const FTextureRHIRef TextureRHI = PostprocessAsset->FlareGradient->Resource->TextureRHI;
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

		MipmapsDownSamples.Empty();
		MipMapUpSamples.Empty();
	}
	// Reset Texture lists
	Output.Texture = MixTexture;
	Output.ViewRect = MixViewport;
}

FScreenPassTexture UQxBloomSubsystem::RenderBloom(FRDGBuilder& GraphBuilder, const FViewInfo& View,
	const FScreenPassTexture& SceneColor, int32 PassAmount)
{
	check(SceneColor.IsValid());

	/// PassAmount <= 1 后面的upsample 等逻辑不适用
	if (PassAmount <= 1)
	{
		return FScreenPassTexture();
	}

	RDG_EVENT_SCOPE(GraphBuilder, "BloomPass");

#pragma region DownSample
	// Downsample
	int32 Width = View.ViewRect.Width();
	int32 Height = View.ViewRect.Height();
	int32 Divider = 2;
	FRDGTextureRef PreviousTexture = SceneColor.Texture;

	for (int32 i = 0; i < PassAmount; ++i)
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

		FRDGTextureRef Texture = nullptr;

		// SceneColor Input 是引擎已经做过down sampleddd的，直接用
		if (i == 0)
		{
			Texture = PreviousTexture;
		}
		else
		{
			Texture = RenderDownSample(
				GraphBuilder,
				PassName,
				View,
				PreviousTexture,
				Size
				);

			FScreenPassTexture DownsampleTexture(Texture, Size);

			MipmapsDownSamples.Add(DownsampleTexture);
			PreviousTexture = Texture;
			
			Divider *= 2;
		}
	}
#pragma endregion

#pragma region UpSample
	float Radius = CVarBloomRadius.GetValueOnRenderThread();

	// downsamples 的结果拷贝到upsamples中以备后续访问
	MipMapUpSamples.Append(MipmapsDownSamples);

	// Stars at -2 since we need the last buffer
	// as the previous input (-2) and the one just
	// before as the current input (-1).
	// We also go from end to start of array to
	// go from small to big texture (going back up the mips)
	for (int i = PassAmount - 3; i >= 0; --i)
	{
		FIntRect CurrentSize = MipMapUpSamples[i].ViewRect;

		const FString PassName  = "UpsampleCombine_"
								+ FString::FromInt( i )
								+ "_"
								+ FString::FromInt( CurrentSize.Width() )
								+ "x"
								+ FString::FromInt( CurrentSize.Height() );

		FRDGTextureRef ResultTexture = RenderUpsampleCombine(
			GraphBuilder,
			PassName,
			View,
			MipMapUpSamples[i], // Current Texture
			MipMapUpSamples[i + 1], // Previous Texture
			Radius
			);

		FScreenPassTexture NewTexture(ResultTexture, CurrentSize);
		MipMapUpSamples[i] = NewTexture;
	}
#pragma endregion

	return MipMapUpSamples[0];
}

FRDGTextureRef UQxBloomSubsystem::RenderDownSample(FRDGBuilder& GraphBuilder, const FString& PassName,
	const FViewInfo& View, FRDGTextureRef InputTexture, const FIntRect& Viewport)
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
	TShaderMapRef<FDownSamplePS_Bloom> PixelShader(View.ShaderMap);

	FDownSamplePS_Bloom::FParameters* PassParams = GraphBuilder.AllocParameters<FDownSamplePS_Bloom::FParameters>();
	PassParams->Pass.InputTexture = InputTexture;
	PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(
		TargetTexture,
		ERenderTargetLoadAction::ENoAction
		);
	PassParams->InputTextureSampler = BilinearBorderSampler;
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

FRDGTextureRef UQxBloomSubsystem::RenderUpsampleCombine(FRDGBuilder& GraphBuilder, const FString& PassName,
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
	TShaderMapRef<FUpsampleCombinePS> PixelShader(View.ShaderMap);

	FUpsampleCombinePS::FParameters* PassParams = GraphBuilder.AllocParameters<FUpsampleCombinePS::FParameters>();
	PassParams->Pass.InputTexture = InputTexture.Texture;
	PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(TargetTexture, ERenderTargetLoadAction::ENoAction);
	PassParams->Radius = Radius;
	PassParams->InputTextureSampler = BilinearClampSampler;
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






