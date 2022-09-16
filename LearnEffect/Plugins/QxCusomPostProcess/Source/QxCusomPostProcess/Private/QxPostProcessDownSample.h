#pragma once
#include "QxCommonShaders.h"
#include "QxPostProcessEyeAdaptation.h"
#include "ScreenPass.h"


class FQxDownSampleChain
{
public:
	// the number of total stages in the chain, 1/64 reduction
	static const uint32 StageCount = 6;
	FQxDownSampleChain() = default;

	// 这个init 会添加downsample chain需要的多个pass，已经log luma需要的pass
	void Init(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& View,
		const FQxEyeAdaptationParameters& EyeAdaptationParameters,
		const FScreenPassTexture& HalfResolutionSceneColor,
		EQxDownampleQuality DownampleQuality,
		bool bLogLumaInAlpha //输出log luminance到alpha通道
		);

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
	
// 添加一个down sample pass需要的参数
struct FQxDownSampleInputs
{
	FQxDownSampleInputs() =default;

	// pas 名
	const TCHAR* Name = TEXT("UnInitialized Downsample Tex");

	// input rdg texture,一定不能是空
	FScreenPassTexture SceneColor;

	EQxDownampleQuality Quality = EQxDownampleQuality::Low;

	// 可以指定这个覆盖output texture格式,unkown 使用scene color的格式
	EPixelFormat FormatOverride = PF_Unknown;
};


namespace QxRenderUtils
{
	FScreenPassTexture AddQxDownSamplePass(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FQxDownSampleInputs& DownSampleInputs
		);
}