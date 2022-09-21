// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderUtils.h"

#include "QxShaders.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"


// DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("qx stat count"), STAT_COUNT, STATGROUP_QxRender);
DECLARE_CYCLE_STAT(TEXT("QxAverageCS"), STAT_QxAverageCS, STATGROUP_QxRender);
DECLARE_CYCLE_STAT(TEXT("QxAverageCS_RHIWay"), STAT_QxAverageCS_RHI, STATGROUP_QxRender);


TArray<FVector> FQxRenderUtils::GetVertexPositonsWS(UStaticMeshComponent* InMeshComponent)
{
	TArray<FVector> meshVerticesWS;
	if (!InMeshComponent)
	{
		return meshVerticesWS;
	}

	UStaticMesh* staticMesh = InMeshComponent->GetStaticMesh();
	if (!staticMesh)
	{
		return  meshVerticesWS;
	}

	// 检查mesh 是否有lod
	if (false == (staticMesh->GetRenderData()->LODResources.Num() > 0))
	{
		return meshVerticesWS;
	}

	// Get the vertices
	FPositionVertexBuffer* vertexBuffer =  &(staticMesh->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer);
	for (uint32 i = 0; i < vertexBuffer->GetNumVertices(); ++i)
	{
		FVector vertexLS = vertexBuffer->VertexPosition(i);

		FVector vertexWS = InMeshComponent->GetComponentTransform().TransformPosition(vertexLS);

		meshVerticesWS.Add(vertexWS);
	}
	return  meshVerticesWS;
}

bool FQxRenderUtils::RayCastHit(const FVector& RayOrigin, const FVector& RayDirection, float RayMarchingLength,
	FHitResult& OutHitResult, AActor* InActor)
{
	const TArray<AActor*> IgActor;

	FVector StartPos = RayOrigin;
	FVector EndPos = RayOrigin + RayDirection * RayMarchingLength;

	return UKismetSystemLibrary::LineTraceSingle(
			InActor,
			StartPos,
			EndPos,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			IgActor,
			EDrawDebugTrace::Type::None,
			OutHitResult,
			true,
			FLinearColor::Blue,
			FLinearColor::Red,
			1.0f
		);
}

void QxRenderUtils::RenderAverageToTarget_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	UTexture2D* InTexture,
	UTextureRenderTarget2D* InRenderTarget)
{
	check(IsInRenderingThread());
	// 确保InRenderTarget的size是InTexture的size / 32
	check(InTexture->GetSizeX()/32 == InRenderTarget->SizeX);

	// RHICmdList.SetCurrentStat()
	// RHICmdList.SetCurrentStat()
	FRDGBuilder GraphBuilder(RHICmdList);
	// 渲染完成后暂存到这个pooled target 中
	TRefCountPtr<IPooledRenderTarget> TmpTarget;
	{
		// RDG_EVENT_SCOPE(GraphBuilder, "QxTestRDG_GroupEvent");
		SCOPE_CYCLE_COUNTER(STAT_QxAverageCS);
		RHICmdList.SetCurrentStat(GET_STATID(STAT_QxAverageCS_RHI));
		FQxAverageCS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxAverageCS::FParameters>();

		// 分配一个临时rdg texture 作为render target
		FRDGTextureDesc TexDesc;
		TexDesc.Extent = FIntPoint(InRenderTarget->SizeX, InRenderTarget->SizeY);
		TexDesc.Format = InRenderTarget->GetFormat();
		TexDesc.Flags = ETextureCreateFlags::TexCreate_UAV | ETextureCreateFlags::TexCreate_Transient;
		TexDesc.ClearValue = FClearValueBinding(FLinearColor::Black);
		
		FRDGTextureRef TmpRDGTexture = GraphBuilder.CreateTexture(TexDesc, TEXT("TmpAverageTarget"));
		FRDGTextureUAVDesc TexUAVDesc(TmpRDGTexture);
		PassParams->OutputTexture = GraphBuilder.CreateUAV(TexUAVDesc);

		PassParams->InputTexture = InTexture->Resource->TextureRHI;
		// PassParams->OutputTexture = RHICmdList.CreateUnorderedAccessView(InRenderTarget->Resource->TextureRHI, 0);
		// RHICmdList.Transition(FRHITransitionInfo(InTexture->Resource->TextureRHI, ERHIAccess::Unknown,
		// 		ERHIAccess::SRVCompute));
		// RHICmdList.Transition(FRHITransitionInfo(InRenderTarget->Resource->TextureRHI, ERHIAccess::Unknown,
		// 		ERHIAccess::UAVCompute));
		
		FIntVector GroupCount = FIntVector(InRenderTarget->SizeX, InRenderTarget->SizeY, 1);
		TShaderMapRef<FQxAverageCS> AverageCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("QxAveragePas"),
			AverageCS,
			PassParams,
			GroupCount
			);

		GraphBuilder.QueueTextureExtraction(TmpRDGTexture, &TmpTarget);
	}
	GraphBuilder.Execute();

	RHICmdList.CopyToResolveTarget(
		TmpTarget.GetReference()->GetRenderTargetItem().TargetableTexture,
		InRenderTarget->GetRenderTargetResource()->TextureRHI,
		FResolveParams()
		);
}

void QxRenderReference::GetRenderStates()
{
	FRHIBlendState* ClearBlendState = TStaticBlendState<>::GetRHI();
	FRHIBlendState* AdditiveBlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();

	FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FRHISamplerState* BilinearBorderSampler = TStaticSamplerState<SF_Bilinear, AM_Border, AM_Border, AM_Border>::GetRHI();
	FRHISamplerState* BilinearRepeatSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
	FRHISamplerState* NearestRepeatSampler = TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
}

FRDGTextureRef QxRenderReference::CreateRDGTexture(FRDGBuilder& GraphBuilder, FRDGTextureRef InputTexture)
{
	// 注册外部的Texture作为RDGTexure
	{
		TRefCountPtr<IPooledRenderTarget> testTexture;
		GraphBuilder.RegisterExternalTexture(
			testTexture,
			TEXT("BlackDummy")
			);
	}

	// Build Texture
	FRDGTextureDesc Desc = InputTexture->Desc; // InputTexture作为输入参数的模板
	Desc.Reset();
	Desc.Extent = FIntPoint(1024, 1024); // 纹理大小 
	Desc.Format = PF_FloatRGB;
	Desc.ClearValue = FClearValueBinding(FLinearColor::Black);
	FRDGTextureRef TargetTexture = GraphBuilder.CreateTexture(Desc,
		TEXT("TestTexture"));
	return TargetTexture;
}
