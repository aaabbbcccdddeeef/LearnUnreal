// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderBPLib.h"
#include "QxRenderSeries.h"
#include <Engine/TextureRenderTarget2D.h>
#include "QxShaders.h"
//#include <Templates/UnrealTemplate.h>

#pragma region PrepareTestDatas
struct FQxTestVertex
{
	FVector4 Position;
	FVector2D UV;
};

// 定义顶点声明
class FQxTestVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	void InitRHI() override
	{
		FVertexDeclarationElementList elements;
		uint32 stride = sizeof(FQxTestVertex);
		elements.Add(FVertexElement(0, STRUCT_OFFSET(FQxTestVertex, Position), VET_Float4, 0, stride));
		elements.Add(FVertexElement(0, STRUCT_OFFSET(FQxTestVertex, UV), VET_Float2, 1, stride));

		VertexDeclarationRHI = RHICreateVertexDeclaration(elements);
	}

	void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}

};

// 定义一个长方形的static vertex buffer
class FSimpleScreenVertexBuffer : public FVertexBuffer
{
public:
	

	void InitRHI() override
	{
		TResourceArray<FQxTestVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(6);

		Vertices[0].Position = FVector4(-1, 1, 0, 1);
		Vertices[0].UV = FVector2D(0, 0);

		Vertices[1].Position = FVector4(1, 1, 0, 1);
		Vertices[1].UV = FVector2D(1, 0);

		Vertices[2].Position = FVector4(-1, -1, 0, 1);
		Vertices[2].UV = FVector2D(0, 1);

		Vertices[3].Position = FVector4(1, -1, 0, 1);
		Vertices[3].UV = FVector2D(1, 1);

		FRHIResourceCreateInfo createInfo(&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, createInfo);
	}

};

TGlobalResource<FQxTestVertexDeclaration> GQxTestVertexDeclaration;
TGlobalResource<FSimpleScreenVertexBuffer> GSimpleScreenVertexBuffer;


//索引缓冲区
class FSimpleIndexBuffer : public FIndexBuffer
{
public:
	void InitRHI() override
	{
		const uint16 Indices[] = { 0, 1, 2 };

		TResourceArray <uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

TGlobalResource<FSimpleIndexBuffer> GSimpleIndexBuffer;
#pragma endregion



void RenderMyTest1(FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutRTResource,
	ERHIFeatureLevel::Type InFeatureLevel,
	FLinearColor InColor)
{
	FGlobalShaderMap* globalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	TShaderMapRef<FQxShaderTestVS> vertexShader(globalShaderMap);
	TShaderMapRef<FQxShaderTestPS> pixelShader(globalShaderMap);

	// 设置Graphic pipeline state
	FGraphicsPipelineStateInitializer graphicPSOInit;
	RHICmdList.ApplyCachedRenderTargets(graphicPSOInit);
	graphicPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	graphicPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	graphicPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	graphicPSOInit.PrimitiveType = PT_TriangleList;

	//设置顶点声明
	graphicPSOInit.BoundShaderState.VertexDeclarationRHI = GQxTestVertexDeclaration.VertexDeclarationRHI;


	graphicPSOInit.BoundShaderState.VertexShaderRHI = vertexShader.GetVertexShader();
	graphicPSOInit.BoundShaderState.PixelShaderRHI = pixelShader.GetPixelShader();
	SetGraphicsPipelineState(RHICmdList, graphicPSOInit);

	pixelShader->SetTestColor(RHICmdList, InColor);

	//RHICmdList.SetStreamSource(0, )
	RHICmdList.SetStreamSource(0, GSimpleScreenVertexBuffer.VertexBufferRHI, 0);

	RHICmdList.DrawIndexedPrimitive(
		GSimpleIndexBuffer.IndexBufferRHI,
		/*BaseVertexIndex=*/ 0,
		/* MinIndex = */  0,
		/* NumVertices = */3,
		/*StartIndex=*/0,
		/* NumPrimitives = */1,
		/*NumInstances=*/1);
}

void DrawQxShaderTestToRT_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutRTResource,
	ERHIFeatureLevel::Type InFeatureLevel,
	FLinearColor InColor)
{
	check(IsInRenderingThread());

	FRHITexture2D* rtRHITexture = OutRTResource->GetRenderTargetTexture();

	RHICmdList.Transition(FRHITransitionInfo(rtRHITexture, ERHIAccess::SRVMask, ERHIAccess::RTV));

	FRHIRenderPassInfo rpInfo(rtRHITexture, ERenderTargetActions::DontLoad_Store);

	RHICmdList.BeginRenderPass(rpInfo, TEXT("QxShaderTest"));
#pragma region MyRegion
	RenderMyTest1(RHICmdList, OutRTResource, InFeatureLevel, InColor);
#pragma endregion

	
	RHICmdList.EndRenderPass();
	RHICmdList.Transition(FRHITransitionInfo(rtRHITexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
}



void UQxRenderBPLib::DrawQxShaderTestToRT(
	UTextureRenderTarget2D* OutRenderTarget, 
	AActor* InActor, 
	FLinearColor InColor)
{
	check(IsInGameThread());

	if (!(OutRenderTarget && InActor))
	{
		UE_LOG(QxRender, Warning, TEXT("content from function %s"));
		return;
	}

	FTextureRenderTargetResource* rtResource =
		OutRenderTarget->GameThread_GetRenderTargetResource();

	UWorld* world = InActor->GetWorld();
	ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(QxDrawTest)(
		[rtResource, featureLevel, InColor](FRHICommandListImmediate& RHICmdList)
		{
			DrawQxShaderTestToRT_RenderThread(RHICmdList, rtResource, featureLevel, InColor);
		}
		);
}
