#pragma once
#include "ScreenPass.h"


UENUM()
enum class EQxAutoExposureMethod : uint8
{
	/** requires compute shader to construct 64 bin histogram */
	AEM_Histogram  UMETA(DisplayName = "Auto Exposure Histogram"),
	/** faster method that computes single value by downsampling */
	AEM_Basic      UMETA(DisplayName = "Auto Exposure Basic"),
	/** Uses camera settings. */
	AEM_None   UMETA(DisplayName = "No auto exposure"),
	AEM_MAX,
};

BEGIN_SHADER_PARAMETER_STRUCT(FQxEyeAdaptationParameters,)
	SHADER_PARAMETER(float, MinAverageLuminance)
	SHADER_PARAMETER(float, MaxAverageLuminance)
	SHADER_PARAMETER(float, ExposureCompensationSettings)
	SHADER_PARAMETER(float, ExposureCompensationCuve)
	SHADER_PARAMETER(float, DeltaWorldTime)
	SHADER_PARAMETER(float, HistogramScale)
	SHADER_PARAMETER(float, HistogramBias)
	SHADER_PARAMETER(float, LuminanceMin)
	SHADER_PARAMETER(float, GreyMult)
	SHADER_PARAMETER(float, LuminanceMax)
	SHADER_PARAMETER_TEXTURE(Texture2D, MeterMaskTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, MeterMaskTextureSampler)
END_SHADER_PARAMETER_STRUCT()

namespace QxRenderUtils
{
	FScreenPassTexture AddBasicEyeAdaptationSetupPass(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& View,
		const FQxEyeAdaptationParameters& EyeAdaptationParameters,
		FScreenPassTexture SceneColor
		);
}
