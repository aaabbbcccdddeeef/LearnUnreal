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


}

IMPLEMENT_GLOBAL_SHADER(FQxScreenPassVS, "/QxPPShaders/QxScreenPass.usf", "QxScreenPassVS", SF_Vertex);

FScreenPassTexture FQxBloomSceneViewExtension::RenderBloomFlare(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FPostProcessMaterialInputs& PostProcessMaterialInput, const UQxBloomFlareAsset* QxBloomSettingAsset)
{

	// const FViewInfo& ViewInfo = dynamic_cast<FViewInfo>(View);
	RDG_GPU_STAT_SCOPE(GraphBuilder, PostProcessQx);
	RDG_EVENT_SCOPE(GraphBuilder, "PostProcessQx");

	int32 DownSamplePassNum = QxPostprocessSubsystem->GetBloomSettingAsset()->DownSampleCount;
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

	FScreenPassTexture MixTexture;

	// #TODO 先测试bloom
	MixTexture = BloomTexture;
	return MixTexture;
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


