#pragma once
#include "ShaderParameterStruct.h"
#include "PostProcess/PostProcessDownsample.h"
#include "PostProcess/PostProcessEyeAdaptation.h"

// 这个文件中主要用来定义一些通用的shader，如downsample的shader等等
enum EQxDownampleQuality
{
	Low, //single filtered sample (2x2 tap)
	High, // four filtered sample (4x4 tap)
	MAX
};

BEGIN_SHADER_PARAMETER_STRUCT(FQxDownsampleParameters,)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Input)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Output)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
END_SHADER_PARAMETER_STRUCT()

// 移植PostProcessDownsample.cpp中的FDownSamplePS
class FQxNormalDownSamplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FQxNormalDownSamplePS);
	SHADER_USE_PARAMETER_STRUCT(FQxNormalDownSamplePS, FGlobalShader);

	class FDownsampleQualityDimension : SHADER_PERMUTATION_ENUM_CLASS("DOWNSAMPLE_QUALITY", EQxDownampleQuality);

	using FPermutationDomain = TShaderPermutationDomain<FDownsampleQualityDimension>;
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_STRUCT_INCLUDE(FQxDownsampleParameters, Common)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};