#include "QxPostProcessEyeAdaptation.h"

#include "QxBloomSceneViewExtension.h"
#include "QxCommonShaders.h"
#include "QxLensFlareAsset.h"
#include "QxPostprocessSubsystem.h"
#include "Interfaces/IPluginManager.h"

#include "RenderGraph.h"
#include "ScreenPass.h"
#include "SystemTextures.h"

namespace
{
	
class FQxDownSampleChain
{
public:
	// the number of total stages in the chain, 1/64 reduction
	static const uint32 StageCount = 6;
	FQxDownSampleChain() = default;

	void Init(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& View,
		EQxDownampleQuality DownampleQuality,
		bool bLogLumaInAlpha //输出log luminance到alpha通道
		)
	{
		RDG_EVENT_SCOPE(GraphBuilder, "QxSceneDownSample");
		static const TCHAR* PassNames[StageCount] =
		{
			TEXT("Scene_Half")
			TEXT("Scene(1/4)"),
			TEXT("Scene(1/8)"),
			TEXT("Scene(1/16)"),
			TEXT("Scene(1/32)"),
			TEXT("Scene(1/64)")
		};
		static_assert(UE_ARRAY_COUNT(PassNames) == StageCount, "Passname size must match stage count");

		for (uint32 StageIndex = 0; StageIndex < StageCount; ++StageIndex)
		{
			FQxNormalDownSamplePS test;
			
		}
	}

	bool IsInitilized() const
	{
		return bIsInitilized;
	}

	FScreenPassTexture GetTexture(uint32 StagetIndex) const
	{
		return Textures[StagetIndex];
	}

	FScreenPassTexture GetFirstTexture() const
	{
		return  Textures[0];
	}
	FScreenPassTexture GetLastTexture() const
	{
		return Textures[StageCount - 1];
	}

private:
	TStaticArray<FScreenPassTexture, StageCount> Textures;
	bool bIsInitilized = false;
};
	
}


FScreenPassTexture FQxBloomSceneViewExtension::RenderEyeAdaptation(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& ViewInfo,
	const FPostProcessMaterialInputs& PostProcessMaterialInput)
{
	FScreenPassTexture ResEyeTexture;
	// ResEyeTexture.ViewRect

		
}

FScreenPassTexture FQxBloomSceneViewExtension::AddDownSamplePass(FRDGBuilder& GraphBuilder, const FViewInfo& ViewInfo,
	const FPostProcessMaterialInputs& PostProcessMaterialInput)
{
	
}


