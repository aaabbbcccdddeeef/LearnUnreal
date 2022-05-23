// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//#include "QxShaders.generated.h"

/**
 * 
 */


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

private:
	// 定义一个输出参数
	LAYOUT_FIELD(FShaderParameter, TestColor);
};