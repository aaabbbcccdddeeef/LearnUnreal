// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPostprocessSubsystem.h"

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
				SetScreenPassPipelineState(PipelineState);

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
	

	// TODO SHADER KAWASE

	// TODO SHADER CHROMA

	// TODO SHADER GHOSTS

	// TODO SHADER HALO

	// TODO SHADER GLARE

	// TODO SHADER MIX
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

	// Final out
	Outputs.Texture = OutputTexture;
	Outputs.Rect = OutputRect;
#pragma endregion
}

FRDGTextureRef UQxPostprocessSubsystem::RenderThreshold(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
	FIntRect& InputRect, const FViewInfo& View)
{
}

FRDGTextureRef UQxPostprocessSubsystem::RenderFlare(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
                                                    FIntRect& InputRect, const FViewInfo& View)
{
}

FRDGTextureRef UQxPostprocessSubsystem::RenderGlare(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
                                                    FIntRect& InputRect, const FViewInfo& View)
{
}

FRDGTextureRef UQxPostprocessSubsystem::RenderBlur(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture,
                                                   const FViewInfo& View, const FIntRect& Viewport, int BlurSteps)
{
}
