// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <ShaderParameterMacros.h>
#include "Shader.h"
#include "ShaderParameters.h"
#include "ShaderParameterStruct.h"
#include "QxShaders.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMyUniformData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxRendering")
		FVector4 ColorOne;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxRendering")
		FVector4 ColorTwo;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxRendering")
		FVector4 ColorThree;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxRendering")
		FVector4 ColorFour;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxRendering")
		int32 ColorIndex;
};


class FQxShaderTestVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FQxShaderTestVS, Global)
public:
	FQxShaderTestVS()
	{
	}

	FQxShaderTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{

	}

	static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("MY_DEFINE"), 1);
	}
};


class FQxShaderTestPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FQxShaderTestPS, Global)

public:
	FQxShaderTestPS()
	{

	}

	FQxShaderTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		TestColor.Bind(Initializer.ParameterMap, TEXT("TestColor"));
		TestTexture.Bind(Initializer.ParameterMap, TEXT("MyTexture"));
		TestTextureSampler.Bind(Initializer.ParameterMap, TEXT("MyTextureSampler"));
	}

	static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("MY_DEFINE"), 1);
	}

	void SetTestColor(FRHICommandList& RHICmdList, const FLinearColor& InTestColor);

	void SetTestTexture(FRHICommandList& RHICmdList, FTextureReferenceRHIRef InTextureRHI);

	void SetTestTexture(FRHICommandList& RHICmdList, FTextureRHIRef  InTextureRHI);

	void SetMyUniform(FRHICommandList& RHICmdList, FMyUniformData InTextureRHI);

	
private:
	// 定义一个输出参数
	LAYOUT_FIELD(FShaderParameter, TestColor);

	LAYOUT_FIELD(FShaderResourceParameter, TestTexture);
	LAYOUT_FIELD(FShaderResourceParameter, TestTextureSampler);
};

#pragma region DefineMyUniformBuffers

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniform, )
SHADER_PARAMETER(FVector4, ColorOne)
SHADER_PARAMETER(FVector4, ColorTwo)
SHADER_PARAMETER(FVector4, ColorThree)
SHADER_PARAMETER(FVector4, ColorFour)
SHADER_PARAMETER(int32, ColorIndex)
END_GLOBAL_SHADER_PARAMETER_STRUCT()


#pragma endregion


#pragma region DefineMyComputeShader
class FQxCheckboardComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FQxCheckboardComputeShader, Global)
public:

	FQxCheckboardComputeShader()
	{

	}

	FQxCheckboardComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		//#Unkown  这里和shader里的名字不对应， 不会有问题吗???
		OutputSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
	}

	static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment)
	{
		//OutEnvironment.SetDefine(TEXT("MY_DEFINE"), 1);
	}

	void SetParameters(FRHICommandList& RHICmdList,
		FTexture2DRHIRef& InOutSurfaceValue,
		FUnorderedAccessViewRHIRef& UAV)
	{
		FRHIComputeShader* shaderRHI = RHICmdList.GetBoundComputeShader();

		// ERHIAccess::
		RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier,
			EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.SetTexture(RHICmdList, shaderRHI, InOutSurfaceValue, UAV);

	}

	void UnsetParameters(FRHICommandList& RHICmdList,
		FUnorderedAccessViewRHIRef& UAV)
	{
		RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable,
			EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.UnsetUAV(RHICmdList, RHICmdList.GetBoundComputeShader());
	}

private:
	LAYOUT_FIELD(FRWShaderParameter, OutputSurface);
};

#pragma endregion


class FQxScreenPassVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQxScreenPassVS);
public:
	FQxScreenPassVS() {  }
	FQxScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	};
	
	static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// 将输入的texture 求平均值输出到out texture,请提前保证输出texture的纬度正确
class FQxAverageCS : public FGlobalShader
{
public:
	static constexpr int32 TGSize_ = 16;
public:
	DECLARE_GLOBAL_SHADER(FQxAverageCS)
	SHADER_USE_PARAMETER_STRUCT(FQxAverageCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_TEXTURE(Texture2D<float>, InputTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputTexture)
		// SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		// OutEnvironment.SetDefine(TEXT("COMPUTE_SHADER"), 1);
		// 注意：测试，理论上来说1024会比用16要差，但实际中差不多，可能是数量级不够????
		// OutEnvironment.SetDefine(TEXT("TGSize_"), 16);
		// OutEnvironment.SetDefine(TEXT("TGSize_"), 1024);
		OutEnvironment.SetDefine(TEXT("TGSize_"), TGSize_);
	}
};
