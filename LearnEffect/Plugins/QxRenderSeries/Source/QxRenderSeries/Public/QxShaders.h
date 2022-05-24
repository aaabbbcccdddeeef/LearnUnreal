// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <ShaderParameterMacros.h>
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
