// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderBPLib.h"
#include "QxRenderSeries.h"
#include <Engine/TextureRenderTarget2D.h>
#include "QxShaders.h"
//#include <Templates/UnrealTemplate.h>

#define LOCTEXT_NAMESPACE "QxRenderBPLib"

#pragma region PrepareTestDatas
struct FQxTestVertex
{
	FVector4 Position;
	FVector2D UV;
};

// ���嶥������
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

// ����һ�������ε�static vertex buffer
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


//����������
class FSimpleIndexBuffer : public FIndexBuffer
{
public:
	void InitRHI() override
	{
		const uint16 Indices[] = { 0, 1, 2, 2, 1, 3 };

		TResourceArray <uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = UE_ARRAY_COUNT(Indices);
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
	FLinearColor InColor,
	FTextureReferenceRHIRef InTextureRHI,
	FMyUniformData InMyUniformData)
{
	FGlobalShaderMap* globalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	TShaderMapRef<FQxShaderTestVS> vertexShader(globalShaderMap);
	TShaderMapRef<FQxShaderTestPS> pixelShader(globalShaderMap);

	// ����Graphic pipeline state
	FGraphicsPipelineStateInitializer graphicPSOInit;
	RHICmdList.ApplyCachedRenderTargets(graphicPSOInit);
	graphicPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	graphicPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	graphicPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	graphicPSOInit.PrimitiveType = PT_TriangleList;

	//���ö�������
	graphicPSOInit.BoundShaderState.VertexDeclarationRHI = GQxTestVertexDeclaration.VertexDeclarationRHI;


	graphicPSOInit.BoundShaderState.VertexShaderRHI = vertexShader.GetVertexShader();
	graphicPSOInit.BoundShaderState.PixelShaderRHI = pixelShader.GetPixelShader();
	SetGraphicsPipelineState(RHICmdList, graphicPSOInit);

	pixelShader->SetTestColor(RHICmdList, InColor);
	pixelShader->SetTestTexture(RHICmdList, InTextureRHI);
	pixelShader->SetMyUniform(RHICmdList, InMyUniformData);

	//RHICmdList.SetStreamSource(0, )
	RHICmdList.SetStreamSource(0, GSimpleScreenVertexBuffer.VertexBufferRHI, 0);

	RHICmdList.DrawIndexedPrimitive(
		GSimpleIndexBuffer.IndexBufferRHI,
		/*BaseVertexIndex=*/ 0,
		/* MinIndex = */  0,
		/* NumVertices = */6,
		/*StartIndex=*/0,
		/* NumPrimitives = */2,
		/*NumInstances=*/1);
}

void DrawQxShaderTestToRT_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutRTResource,
	ERHIFeatureLevel::Type InFeatureLevel,
	FLinearColor InColor,
	FTextureReferenceRHIRef InTextureRHI, 
	FMyUniformData InMyUniformData)
{
	check(IsInRenderingThread());

	FRHITexture2D* rtRHITexture = OutRTResource->GetRenderTargetTexture();

	RHICmdList.Transition(FRHITransitionInfo(rtRHITexture, ERHIAccess::SRVMask, ERHIAccess::RTV));

	FRHIRenderPassInfo rpInfo(rtRHITexture, ERenderTargetActions::DontLoad_Store);

	RHICmdList.BeginRenderPass(rpInfo, TEXT("QxShaderTest"));
#pragma region MyRegion
	RenderMyTest1(RHICmdList, OutRTResource, InFeatureLevel, InColor, InTextureRHI, InMyUniformData);
#pragma endregion

	
	RHICmdList.EndRenderPass();
	RHICmdList.Transition(FRHITransitionInfo(rtRHITexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
}



void UQxRenderBPLib::DrawQxShaderTestToRT(
	UTextureRenderTarget2D* OutRenderTarget, 
	AActor* InActor, 
	FLinearColor InColor,
	UTexture* MyTexture,
	FMyUniformData InMyUniformData)
{
	check(IsInGameThread());

	if (!(OutRenderTarget && InActor))
	{
		UE_LOG(QxRender, Warning, TEXT("content from function %s"));
		return;
	}

	FTextureRenderTargetResource* rtResource =
		OutRenderTarget->GameThread_GetRenderTargetResource();
	FTextureReferenceRHIRef myTextureRHI = MyTexture->TextureReference.TextureReferenceRHI;

	UWorld* world = InActor->GetWorld();
	ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(QxDrawTest)(
		[rtResource, featureLevel, InColor, myTextureRHI, InMyUniformData](FRHICommandListImmediate& RHICmdList)
		{
			DrawQxShaderTestToRT_RenderThread(RHICmdList, rtResource, featureLevel, InColor, myTextureRHI, InMyUniformData);
		}
		);
}




void UQxRenderBPLib::TextureWriting(UTexture2D* TextureToWrite, AActor* selfref)
{
	check(IsInGameThread());

	if (TextureToWrite == nullptr || selfref == nullptr)
	{
		return;
	}

	TextureToWrite->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	TextureToWrite->SRGB = 0;

	FTexture2DMipMap& mipmap = TextureToWrite->PlatformData->Mips[0];
	void* Data = mipmap.BulkData.Lock(LOCK_READ_WRITE);

	int32 textureX = TextureToWrite->PlatformData->SizeX;
	int32 textureY = TextureToWrite->PlatformData->SizeY;
	TArray<FColor> colors;
	colors.AddUninitialized(textureX * textureY);
	for (int32 i = 0; i < textureX * textureY; i++)
	{
		colors[i] = FColor::Blue;
	}
	int32 stride = (int32)(sizeof(uint8) * 4);
	check((sizeof(uint8) * 4) == sizeof(FColor));
	FMemory::Memcpy(Data, colors.GetData(), textureX * textureY * stride);
	mipmap.BulkData.Unlock();
	TextureToWrite->UpdateResource();

}

void TextureWriting2_RenderThread(FRHICommandListImmediate& RHICmdList,
	ERHIFeatureLevel::Type InFeatureLevel,
	UTexture2D* TextureToWrite)
{
	check(IsInRenderingThread());
	if (TextureToWrite == nullptr)
	{
		return;
	}

	FTextureReferenceRHIRef targetTexRHI = TextureToWrite->TextureReference.TextureReferenceRHI;
	FRHITexture* texRef = targetTexRHI->GetTextureReference()->GetReferencedTexture();
	FRHITexture2D* texRef2D = (FRHITexture2D*)(texRef);

	TArray<FColor> bitmap;
	uint32 LolStride = 0;
	char* textureDataPtr = (char*)RHICmdList.LockTexture2D(texRef2D, 0,
		EResourceLockMode::RLM_ReadOnly, LolStride, false);

	for (uint32  row = 0; row < texRef2D->GetSizeY(); row++)
	{
		uint32* pixelPtr = (uint32*)(textureDataPtr);

		for (uint32 column = 0; column < texRef2D->GetSizeX(); column++)
		{
			uint32 encodePixel = *pixelPtr;
			// Ϊʲô����������ת���� �Լ������Ϊʲô����
			uint8	r = (encodePixel && 0x000000FF);
			uint8 g = (encodePixel && 0x0000FF00) >> 8;
			uint8 b = (encodePixel & 0x00FF0000) >> 16;
			uint8 a = (encodePixel & 0xFF000000) >> 24;

			FColor col = FColor(r, g, b, a);
			bitmap.Add(col);
			pixelPtr++;

		}
		// �ƶ�����һ��
		textureDataPtr += LolStride;
	}

	RHICmdList.UnlockTexture2D(texRef2D, 0, false);

	if (bitmap.Num())
	{
		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);
		const FString screenFileName = FPaths::ScreenShotDir() / TEXT("VisualizeTexture");
		// #Unkown ���������ʲô��˼
		uint32 extendXWithMSAA = TextureToWrite->GetSizeX();
		if (TextureToWrite->GetSizeY() > 0)
		{
			extendXWithMSAA = bitmap.Num() / TextureToWrite->GetSizeY();
		}

		// ������ɫ���鵽һ��bitmap �ļ�
		FFileHelper::CreateBitmap(*screenFileName, extendXWithMSAA, TextureToWrite->GetSizeY(), bitmap.GetData());

		UE_LOG(LogConsoleResponse, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
	}
	else
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
	}
}

void UQxRenderBPLib::TextureWriting2(UTexture2D* TextureToWrite, AActor* selfref)
{
	check(IsInGameThread());
	if (TextureToWrite == nullptr || selfref == nullptr)
	{
		return;
	}

	UWorld* world = selfref->GetWorld();
	ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(QxWriteToTexture)(
		[featureLevel, TextureToWrite](FRHICommandListImmediate& RHICmdList)
		{
			TextureWriting2_RenderThread(RHICmdList, featureLevel, TextureToWrite);
		}
		);
}

void DrawCheckBoard_RenderThread(FRHICommandListImmediate& RHICmdList, 
	FTextureRenderTargetResource* InTextureRTResource,
	ERHIFeatureLevel::Type InFeatureLevel)
{
	check(IsInRenderingThread());

	FTexture2DRHIRef rtTexRHI = InTextureRTResource->GetRenderTargetTexture();
	uint32 GGroupSize = 32;

	FIntPoint fullResolution = FIntPoint(rtTexRHI->GetSizeX(), rtTexRHI->GetSizeY());
	uint32 groupSizeX = FMath::DivideAndRoundUp((uint32)rtTexRHI->GetSizeX(), GGroupSize);
	uint32 groupSizeY = FMath::DivideAndRoundUp((uint32)rtTexRHI->GetSizeY(), GGroupSize);

	TShaderMapRef<FQxCheckboardComputeShader> computeShader(GetGlobalShaderMap(InFeatureLevel));

	RHICmdList.SetComputeShader(computeShader.GetComputeShader());

	FRHIResourceCreateInfo createInfo;

	// Create a tmp resource
	FTexture2DRHIRef gSurfaceTexture2D = RHICreateTexture2D(rtTexRHI->GetSizeX(),
		rtTexRHI->GetSizeY(),
		PF_FloatRGBA, 1, 1, TexCreate_ShaderResource | TexCreate_UAV,
		createInfo);
	FUnorderedAccessViewRHIRef gUAV = RHICreateUnorderedAccessView(gSurfaceTexture2D);

	computeShader->SetParameters(RHICmdList, rtTexRHI, gUAV);
	DispatchComputeShader(RHICmdList, computeShader, groupSizeX, groupSizeY, 1);
	computeShader->UnsetParameters(RHICmdList, gUAV);

	FRHICopyTextureInfo copyInfo;
	RHICmdList.CopyTexture(gSurfaceTexture2D, rtTexRHI, copyInfo);
}


void UQxRenderBPLib::DrawCheckBoard(const UObject* WorldContextObject, UTextureRenderTarget2D* OutRenderTarget)
{
	check(IsInGameThread());

	if (!OutRenderTarget)
	{
		FMessageLog("Blueprint").Warning(
			LOCTEXT("UGraphicToolsBlueprintLibrary::DrawCheckerBoard",
			"DrawUVDisplacementToRenderTarget: Output render target is required."));
		return;
	}

	FTextureRenderTargetResource* textureRTResource = OutRenderTarget->GameThread_GetRenderTargetResource();
	ERHIFeatureLevel::Type featureLevel = WorldContextObject->GetWorld()->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(QxDrawCheckboard)(
		[featureLevel, textureRTResource](FRHICommandListImmediate& RHICmdList)
		{
			DrawCheckBoard_RenderThread(
				RHICmdList, textureRTResource, featureLevel
			);
		}
		);
}