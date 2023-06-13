#include "QxBlurs.h"

#include "PixelShaderUtils.h"
#include "QxCommonShaders.h"
#include "QxPostProcessBloom.h"
#include "QxRenderPPUtils.h"

DECLARE_GPU_STAT(QxGuassian);
namespace 
{
	// Blur shader use Kawase method
	class FQxKawaseBlurDownPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxKawaseBlurDownPS);
		SHADER_USE_PARAMETER_STRUCT(FQxKawaseBlurDownPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(FVector2D, BufferSize)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return  IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxKawaseBlurDownPS, "/QxPPShaders/QxDualKawaseBlur.usf", "KawaseBlurDownsamplePS", SF_Pixel);

	// Blur shader use Kawase method
	class FQxKawaseBlurUpPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxKawaseBlurUpPS);
		SHADER_USE_PARAMETER_STRUCT(FQxKawaseBlurUpPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxBloomFlarePassParameters, Pass)
			SHADER_PARAMETER(FVector2D, BufferSize)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return  IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxKawaseBlurUpPS, "/QxPPShaders/QxDualKawaseBlur.usf", "KawaseBlurUpsamplePS", SF_Pixel);

	class FQxGuassianCS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxGuassianCS);
		SHADER_USE_PARAMETER_STRUCT(FQxGuassianCS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, BluredTexture)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(FGlobalShaderPermutationParameters const& Parameters)
		{
			return RHISupportsComputeShaders(Parameters.Platform) && !IsHlslccShaderPlatform(Parameters.Platform);;
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGuassianCS, "/QxPPShaders/QxGuassianBlur.usf", "MainCS", SF_Compute);
}

FScreenPassTexture QxRenderPPUtils::RenderKawaseBlur(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& ViewInfo,
	const FScreenPassTexture& InputTexture,
	int32 BlurSteps)
{
	// 确保blur 上限不超过 合理的数值
	float MaxStep = FMath::Log2(
		FMath::Min(InputTexture.ViewRect.Width(), InputTexture.ViewRect.Width())
		);
	BlurSteps = FMath::Min<int32>(MaxStep, BlurSteps);
	
	const int32 PassSum = BlurSteps * 2;
	
	// 准备down sample和upsample的不同尺寸的view rects
	int32 Divider = 2;
	TArray<FIntRect> ViewRects;
	ViewRects.AddUninitialized(PassSum);
	for (int32 i = 0; i < PassSum; ++i)
	{
		FIntRect NewRect = FIntRect(
			0, 0,
			InputTexture.ViewRect.Width() / Divider,
			InputTexture.ViewRect.Height() / Divider
			);
		ViewRects[i] = MoveTemp(NewRect);

		// 这里的判断和blur step的意思是blurstep以下是downsample， 以上是upsample
		if (i < (BlurSteps - 1))
		{
			Divider *= 2;
		}
		else
		{
			Divider /= 2;
		}
	}


	FRDGTextureRef PreviousBuffer = InputTexture.Texture;
	TShaderMapRef<FQxKawaseBlurDownPS> KawaseDownPS(ViewInfo.ShaderMap);
	TShaderMapRef<FQxKawaseBlurUpPS> KawaseUpPS(ViewInfo.ShaderMap);
	
	// Render downsamples and upsamples
	for (int32 i = 0; i < PassSum; ++i)
	{
		FRDGTextureDesc BlurTexDesc = InputTexture.Texture->Desc;
		BlurTexDesc.Reset();
		BlurTexDesc.Extent = ViewRects[i].Size();
		BlurTexDesc.Format = PF_FloatRGB;
		BlurTexDesc.NumMips = 1;
		BlurTexDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		

		const FVector2D ViewportResolution = FVector2D(ViewRects[i].Size());

		const FString PassName = FString::Printf(
			TEXT("QxKawaseBlur_%i_%s_%ix%i"),
			i,
			(i < BlurSteps ? TEXT("Down") : TEXT("Up")),
			ViewRects[i].Width(),
			ViewRects[i].Height()
			);
		FRDGTextureRef Buffer = GraphBuilder.CreateTexture(BlurTexDesc, *PassName);
		
		if (i < BlurSteps)
		{
			FQxKawaseBlurDownPS::FParameters* PassDownParameters = GraphBuilder.AllocParameters<FQxKawaseBlurDownPS::FParameters>();
			PassDownParameters->Pass.InputTexture       = PreviousBuffer;
			PassDownParameters->Pass.RenderTargets[0]   = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ENoAction);
			PassDownParameters->Pass.InputTextureSampler   = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
			PassDownParameters->BufferSize              = ViewportResolution;

			// FPixelShaderUtils::AddFullscreenPass(
			// 	GraphBuilder,
			// 	ViewInfo.ShaderMap,
			// 	RDG_EVENT_NAME("%s", *PassName),
			// 	KawaseDownPS,
			// 	PassDownParameters,
			// 	ViewRects[i]
			// 	);
			QxRenderPPUtils::QxDrawScreenPass(
				GraphBuilder,
				PassName,
				KawaseDownPS,
				PassDownParameters,
				ViewRects[i]
				);
		}
		else
		{
			FQxKawaseBlurUpPS::FParameters* PassDownParameters = GraphBuilder.AllocParameters<FQxKawaseBlurUpPS::FParameters>();
			PassDownParameters->Pass.InputTexture       = PreviousBuffer;
			PassDownParameters->Pass.RenderTargets[0]   = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ENoAction);
			PassDownParameters->Pass.InputTextureSampler   = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
			PassDownParameters->BufferSize              = ViewportResolution;

			// FPixelShaderUtils::AddFullscreenPass(
			// 	GraphBuilder,
			// 	ViewInfo.ShaderMap,
			// 	RDG_EVENT_NAME("%s", *PassName),
			// 	KawaseUpPS,
			// 	PassDownParameters,
			// 	ViewRects[i]
			// 	);
			QxRenderPPUtils::QxDrawScreenPass(
				GraphBuilder,
				PassName,
				KawaseUpPS,
				PassDownParameters,
				ViewRects[i]
				);
		}

		PreviousBuffer = Buffer;
	}
	FScreenPassTexture OutputTexture;
	OutputTexture.Texture = PreviousBuffer;
	OutputTexture.ViewRect = ViewRects.Last();
	return OutputTexture;
}

FScreenPassTexture QxRenderPPUtils::RenderQxGuassianBlur(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FScreenPassTexture& InputTexture, bool bUseComputeShader)
{
	RDG_GPU_STAT_SCOPE(GraphBuilder, QxGuassian);
	RDG_EVENT_SCOPE(GraphBuilder, "QxGuassian");

	const FString PassName("QxGuassian");
	
	FRDGTextureRef BluredTexture;

	{
		FRDGTextureDesc Desc = InputTexture.Texture->Desc;
		BluredTexture = GraphBuilder.CreateTexture(Desc, *PassName);
	}

	TShaderMapRef<FQxGuassianCS> ComputeShader(GetGlobalShaderMap(ViewInfo.GetFeatureLevel()));

	FQxGuassianCS::FParameters* PassParameters =
		GraphBuilder.AllocParameters<FQxGuassianCS::FParameters>();
	PassParameters->InputTexture = InputTexture.Texture;
	PassParameters->InputTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	// EPixelFormat OutFormat = InputTexture.Texture->Desc.Format;
	PassParameters->BluredTexture =
		GraphBuilder.CreateUAV(BluredTexture);

	FIntVector GroupCount;
	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("QxGuassianBlur"),
		ComputeShader, PassParameters,
		GroupCount
		);

	FScreenPassTexture OutTexture;
	OutTexture.ViewRect = FIntRect(
		0, 0,
		ViewInfo.ViewRect.Width(),
		ViewInfo.ViewRect.Height()
		);

	return OutTexture;
}
