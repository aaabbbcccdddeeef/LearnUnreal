// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "UObject/NoExportTypes.h"
#include "GlobalShader.h"

#include "ShaderParameterStruct.h"
// #include "QxCSShader_RDG.generated.h"


class QXRENDERSERIES_API FQxCSSahder_RDG : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FQxCSSahder_RDG);

	// 下面这段是RDG shader参数定义的标准格式
	SHADER_USE_PARAMETER_STRUCT(FQxCSSahder_RDG, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FVector>, VertexPositions) //这里要和hlsl shader中的参数完全对应
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()
	
public:
	// FQxCSSahder_RDG() {  }
	//
	// explicit FQxCSSahder_RDG(const FGlobalShaderType::CompiledShaderInitializerType& Initializer)
	// 	: FGlobalShader(Initializer)
	// {
	// 	
	// };

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}


	void BuildAndExecuteRenderGraph_RenderThread(FRHICommandListImmediate& RHICmdList,
	                                UTextureRenderTarget2D* InRenderTarget, TArray<FVector> InVertexPositions);
};
