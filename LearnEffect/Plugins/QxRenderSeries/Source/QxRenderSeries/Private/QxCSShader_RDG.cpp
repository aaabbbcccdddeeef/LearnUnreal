// Fill out your copyright notice in the Description page of Project Settings.


#include "QxCSShader_RDG.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTarget2D.h"

// 将shader C++定义和hlsl shader关联
IMPLEMENT_GLOBAL_SHADER(FQxCSSahder_RDG, "/QxShaders/QxCS_RDG.usf", "MainCompute", SF_Compute);

void FQxCSSahder_RDG::BuildAndExecuteRenderGraph_RenderThread(FRHICommandListImmediate& RHICmdList,
									UTextureRenderTarget2D* InRenderTarget, TArray<FVector> InVertexPositions)
{
	if (!IsInRenderingThread())
	{
		return;
	}
	
	
	FRDGBuilder graphBuilder(RHICmdList);
	// Extracting to pooled render target
	TRefCountPtr<IPooledRenderTarget> pooledComputeTarget;
	// 添加自定义pass 名字
	SCOPED_DRAW_EVENT(RHICmdList, QxTestEvent);
	{
		// RDG的方式添加pass名字
		RDG_EVENT_SCOPE(graphBuilder, "QxTestRDG_GroupEvent");

		// 分配RDG 参数资源
		FParameters* passParameters = graphBuilder.AllocParameters<FQxCSSahder_RDG::FParameters>();

		// 分配rdg管理的 structed buffer
		FRDGBufferRef verticesBuffer = CreateStructuredBuffer(
			graphBuilder,
			TEXT("vertices_strucedbuffer"),
			sizeof(FVector),
			InVertexPositions.Num(),
			InVertexPositions.GetData(),
			sizeof(FVector) * InVertexPositions.Num()
		);
		// 分配rdg管理的srv
		FRDGBufferSRVRef verticesSRV = graphBuilder.CreateSRV(verticesBuffer, EPixelFormat::PF_R32_UINT);
		// 将shader参数和 srv关联
		passParameters->VertexPositions = verticesSRV;

		FRDGTextureDesc outTextureDesc = FRDGTextureDesc::Create2D(
			FIntPoint(InRenderTarget->SizeX, InRenderTarget->SizeY),
			PF_FloatRGBA,
			FClearValueBinding(),
			TexCreate_UAV,
			1,
			1
			);
		FRDGTextureRef outTextureRef = graphBuilder.CreateTexture(outTextureDesc, TEXT("Compute_Out_Texture"));
		FRDGTextureUAVDesc outTextureUAVDesc(outTextureRef);
		// 将uav 和shader参数绑定
		passParameters->OutputTexture = graphBuilder.CreateUAV(outTextureUAVDesc);

		TShaderMapRef<FQxCSSahder_RDG> cs_RDG(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		FComputeShaderUtils::AddPass(
			graphBuilder,
			RDG_EVENT_NAME("QxComputePass"),
			cs_RDG,
			passParameters,
			FIntVector(32, 32, 1) // #TODO 这里为什么这样分配不是很明白
			);
	
	
		graphBuilder.QueueTextureExtraction(outTextureRef, &pooledComputeTarget);
	}
	graphBuilder.Execute();
	
	RHICmdList.CopyToResolveTarget(
		pooledComputeTarget.GetReference()->GetRenderTargetItem().TargetableTexture,
		InRenderTarget->GetRenderTargetResource()->TextureRHI,
		FResolveParams()
	);
	
}