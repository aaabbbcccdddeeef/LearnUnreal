#include "QxPostProcessBloom.h"

#include <stdexcept>

#include "QxBloomSceneViewExtension.h"
#include "QxLensFlareAsset.h"
#include "QxPostprocessSubsystem.h"
#include "Interfaces/IPluginManager.h"

#include "RenderGraph.h"
#include "ScreenPass.h"
#include "SystemTextures.h"

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
}

IMPLEMENT_GLOBAL_SHADER(FQxScreenPassVS, "/QxPPShaders/QxScreenPass.usf", "QxScreenPassVS", SF_Vertex);

FScreenPassTexture FQxBloomSceneViewExtension::RenderBloomFlare(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FPostProcessMaterialInputs& PostProcessMaterialInput, const UQxBloomFlareAsset* QxBloomSettingAsset)
{

	// const FViewInfo& ViewInfo = dynamic_cast<FViewInfo>(View);
	RDG_GPU_STAT_SCOPE(GraphBuilder, PostProcessQx);
	RDG_EVENT_SCOPE(GraphBuilder, "PostProcessQx");

	const UQxBloomFlareAsset* const BloomSettingAsset = QxPostprocessSubsystem->GetBloomSettingAsset();
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
		PostProcessMaterialInput.GetInput(EPostProcessMaterialInput::SceneColor);

		
	// Scene Color Rescal passes,
	// 要处理编辑器viewport的变化

	// Bloom
	{
		BloomTexture = RenderBloom(
			GraphBuilder,
			ViewInfo,
			SceneColorTexture,
			DownSamplePassNum
			);
	}	

	// Flare

	// Glare

	// Composite Bloom, Flare and Glare together

	
	FRDGTextureRef MixTexture;
	FIntRect MixViewport(
		0, 0,
		ViewInfo.ViewRect.Width() / 2,
		ViewInfo.ViewRect.Height() / 2
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
			RenderDownSample(
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


