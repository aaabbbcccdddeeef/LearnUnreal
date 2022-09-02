// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderBPLib.h"
#include "QxRenderSeries.h"
#include <Engine/TextureRenderTarget2D.h>
#include "QxShaders.h"
#include <GameFramework/GameUserSettings.h>

#include "ClearQuad.h"
#include "Renderer/Private/PostProcess/SceneRenderTargets.h"

#include "QxCSShader_RDG.h"
#include "QxRenderUtils.h"
#include "QxSubsystem.h"
#include "RenderTargetPool.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"
//#include <Templates/UnrealTemplate.h>

#define LOCTEXT_NAMESPACE "QxRenderBPLib"

extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

TRefCountPtr<IPooledRenderTarget> UQxRenderBPLib::PooledRenderTarget;

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
	//GDynamicRHI
	//FD3D12DynamicRHI* rhiPtr = dynamic_cast<FD3D12DynamicRHI>(GDynamicRHI);
	//if (rhiPtr)
	//{
	//}

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

	// FGraphicsPipelineState* PipelineState;
	// PipelineState.set
	// graphicPSOInit.set
	FTextureRHIRef TexRHIRef = InTextureRHI->GetReferencedTexture();
	pixelShader->SetTestColor(RHICmdList, InColor);
	pixelShader->SetTestTexture(RHICmdList, TexRHIRef);
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
			// 为什么这里是这样转换， 以及最后结果为什么不对
			uint8	r = (encodePixel && 0x000000FF);
			uint8 g = (encodePixel && 0x0000FF00) >> 8;
			uint8 b = (encodePixel & 0x00FF0000) >> 16;
			uint8 a = (encodePixel & 0xFF000000) >> 24;

			FColor col = FColor(r, g, b, a);
			bitmap.Add(col);
			pixelPtr++;

		}
		// 移动到下一行
		textureDataPtr += LolStride;
	}

	RHICmdList.UnlockTexture2D(texRef2D, 0, false);

	if (bitmap.Num())
	{
		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);
		const FString screenFileName = FPaths::ScreenShotDir() / TEXT("VisualizeTexture");
		// #Unkown 下面这句是什么意思
		uint32 extendXWithMSAA = TextureToWrite->GetSizeX();
		if (TextureToWrite->GetSizeY() > 0)
		{
			extendXWithMSAA = bitmap.Num() / TextureToWrite->GetSizeY();
		}

		// 保存颜色数组到一个bitmap 文件
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

void UQxRenderBPLib::SetTextureForamt(UTexture2D* InTexture, int32 InSizeX, int32 InSizeY)
{
	check(IsInGameThread());
	if (InTexture == nullptr || InSizeX == 0 || InSizeY == 0)
	{
		return;
	}
	// FName InName
	// UTexture2D* NewTexture = NULL;
	EPixelFormat InFormat = EPixelFormat::PF_A32B32G32R32F;
	if (InSizeX > 0 && InSizeY > 0 &&
		(InSizeX % GPixelFormats[InFormat].BlockSizeX) == 0 &&
		(InSizeY % GPixelFormats[InFormat].BlockSizeY) == 0)
	{
		// NewTexture = NewObject<UTexture2D>(
		// 	GetTransientPackage(),
		// 	InName,
		// 	RF_Transient
		// 	);

		// InTexture->PlatformData = new FTexturePlatformData();
		InTexture->PlatformData->SizeX = InSizeX;
		InTexture->PlatformData->SizeY = InSizeY;
		InTexture->PlatformData->PixelFormat = InFormat;
		InTexture->NeverStream = true;
		InTexture->SRGB = 0;
		InTexture->LODGroup = TextureGroup::TEXTUREGROUP_Pixels2D;
		
		//Allocate first mipmap.
		int32 NumBlocksX = InSizeX / GPixelFormats[InFormat].BlockSizeX;
		int32 NumBlocksY = InSizeY / GPixelFormats[InFormat].BlockSizeY;
		// FTexture2DMipMap* Mip = new FTexture2DMipMap();
		// InTexture->PlatformData->Mips.Empty();
		// InTexture->PlatformData->Mips.Add(Mip);
		FTexture2DMipMap& Mip = InTexture->PlatformData->Mips[0];
		Mip.SizeX = InSizeX;
		Mip.SizeY = InSizeY;
		Mip.BulkData.Lock(LOCK_READ_WRITE);
		Mip.BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InFormat].BlockBytes);
		Mip.BulkData.Unlock();

		InTexture->UpdateResource();

		if (InTexture->MarkPackageDirty() == false)
		{
			UE_LOG(LogTemp, Error, TEXT("Modify Texture failed"));
		}
		
	}
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

bool UQxRenderBPLib::GetUseD3D12InGame()
{
	bool bPreferD3D12 = false;
	GConfig->GetBool(TEXT("D3DRHIPreference"), TEXT("bUseD3D12InGame"), bPreferD3D12, GGameUserSettingsIni);

	UE_LOG(LogTemp, Warning, TEXT("content from function "));
	return bPreferD3D12;
}

void UQxRenderBPLib::SetUseD3D12InGame(bool InUseD3D12)
{
	GConfig->SetBool(TEXT("D3DRHIPreference"), TEXT("bUseD3D12InGame"), InUseD3D12, GGameUserSettingsIni);
	//SaveConfig(CPF_Config, *GGameUserSettingsIni);
	UGameUserSettings::GetGameUserSettings()->SaveSettings();
	UE_LOG(LogTemp, Warning, TEXT("content from function "));
}

void UQxRenderBPLib::TestAccessEngineGlobals(float& OutAverageFPS, float& OutAverageMS)
{
	OutAverageFPS = GAverageFPS;
	OutAverageMS = GAverageMS;
}

void UQxRenderBPLib::TestAccessSubsystem(UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	UQxSubsystem* QxSubsystem = GameInstance->GetSubsystem<UQxSubsystem>();
	QxSubsystem->AddScore(10);
}

void UQxRenderBPLib::RenderTexture_WithCSRDG(UTextureRenderTarget2D* InRenderTarget, TArray<FVector> InVertexPositions)
{
	TShaderMapRef<FQxCSSahder_RDG> qxCS_RDG(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	ENQUEUE_RENDER_COMMAND(QxCSRDGRender)(
	[qxCS_RDG, InRenderTarget, InVertexPositions](FRHICommandListImmediate& RHICmdList)
	{
		qxCS_RDG->BuildAndExecuteRenderGraph_RenderThread(
				RHICmdList,
				InRenderTarget,
				InVertexPositions
			);
	}
	);
	
}

TArray<FVector> UQxRenderBPLib::GetMeshVerticesWS(UStaticMeshComponent* InMeshComponent)
{
	return FQxRenderUtils::GetVertexPositonsWS(InMeshComponent);
}

void UQxRenderBPLib::PostResolveSceneColor_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FSceneRenderTargets& SceneRenderTargets)
{
	// Used to gather CPU profiling data for the UE4 session frontend
	// 添加给front end等用的标记
	QUICK_SCOPE_CYCLE_COUNTER(STAT_QxShaderPlugin_PixelShader); 
	// Used to profile GPU activity and add metadata to be consumed by for example RenderDoc
	// 添加给renderdoc等用的标记
	SCOPED_DRAW_EVENT(RHICmdList, QxShaderPlugin_Pixel); 


	//获得 当前render target的color buffer
	FTextureRHIRef rtTextureRHI = SceneRenderTargets.GetSceneColor()->GetRenderTargetItem().ShaderResourceTexture;
		//SceneRenderTargets.GetSceneColorSurface();
	//SceneRenderTargets
	//GRenderTargetPool

	// 请求一个临时RT

	// 原来的RT转换成SRV

	// 绘制

	// 
	{
		//SceneRenderTargets.BeginRenderingPrePass();
		//
	FSceneRenderTargetItem& renderTargete =		RequestSurface(RHICmdList);
	RHICmdList.Transition(FRHITransitionInfo(rtTextureRHI, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		// 从根据SceneColor分配一个IRenderTarget，获得原来的SceneColor RenderTarget, 将新的设置给SceneContext,
		// 转换原来的RenderTarget状态

		
	// FRHIRenderPassInfo rpInfo(renderTargete.TargetableTexture, ERenderTargetActions::DontLoad_Store);
	FRHIRenderPassInfo rpInfo(
		SceneRenderTargets.GetSceneColor()->GetRenderTargetItem().TargetableTexture,
		ERenderTargetActions::DontLoad_Store);


	RHICmdList.BeginRenderPass(rpInfo, TEXT("QxShaderTest"));

	FMyUniformData TestUniformData;
	TestUniformData.ColorOne = FVector4(FLinearColor::Black );
	TestUniformData.ColorTwo = FVector4(FLinearColor::Red);
	TestUniformData.ColorThree = FVector4(FLinearColor::Green);
	TestUniformData.ColorFour = FVector4(FLinearColor::Blue);
	TestUniformData.ColorIndex = 1;

	// FTextureReferenceRHIRef rtRHI = rtTextureRHI;
#pragma region MyRegion
	// RenderMyTest1(RHICmdList, OutRTResource, GMaxRHIFeatureLevel, FLinearColor::Red, FTextureReferenceRHIRef(), TestUniformData);

	FGlobalShaderMap* globalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	TShaderMapRef<FQxShaderTestVS> vertexShader(globalShaderMap);
	TShaderMapRef<FQxShaderTestPS> pixelShader(globalShaderMap);

	// 设置Graphic pipeline state
	FGraphicsPipelineStateInitializer graphicPSOInit;
	RHICmdList.ApplyCachedRenderTargets(graphicPSOInit);
	graphicPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	graphicPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	graphicPSOInit.PrimitiveType = PT_TriangleList;

	bool bAdditiveBlend = false;
	// set the state
	if (bAdditiveBlend)
	{
		graphicPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
	}
	else
	{
		graphicPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	}

	//设置顶点声明
	graphicPSOInit.BoundShaderState.VertexDeclarationRHI = GQxTestVertexDeclaration.VertexDeclarationRHI;

	graphicPSOInit.BoundShaderState.VertexShaderRHI = vertexShader.GetVertexShader();
	graphicPSOInit.BoundShaderState.PixelShaderRHI = pixelShader.GetPixelShader();
	SetGraphicsPipelineState(RHICmdList, graphicPSOInit);

	// FGraphicsPipelineState* PipelineState;
	// PipelineState.set
	// graphicPSOInit.set
	
	pixelShader->SetTestColor(RHICmdList, FLinearColor::Red);
	// pixelShader->SetTestTexture(RHICmdList, rtTextureRHI);
		FTextureRHIRef sceneColorTexRHI1  = SceneRenderTargets.GetSceneColor()->GetRenderTargetItem().ShaderResourceTexture;
		FTextureRHIRef sceneColorTexRHI = SceneRenderTargets.GetSceneColorTexture();
		FTextureRHIRef GBufferCRHI = SceneRenderTargets.GBufferC->GetRenderTargetItem().ShaderResourceTexture;
		FTextureRHIRef TestRHI1 = SceneRenderTargets.GetGBufferATexture();
		pixelShader->SetTestTexture(RHICmdList, sceneColorTexRHI);

	pixelShader->SetMyUniform(RHICmdList, TestUniformData);

	//RHICmdList.SetStreamSource(0, )
	RHICmdList.SetStreamSource(0, GSimpleScreenVertexBuffer.VertexBufferRHI, 0);

	RHICmdList.DrawIndexedPrimitive(
		GSimpleIndexBuffer.IndexBufferRHI,
		 0,
		  0,
		6,
		0,
		2,
		1);
#pragma endregion

	RHICmdList.EndRenderPass();
	}

	auto& tmp = SceneRenderTargets.GetSceneColor();
	RHICmdList.CopyToResolveTarget(
tmp->GetRenderTargetItem().TargetableTexture,
tmp->GetRenderTargetItem().ShaderResourceTexture,
				FResolveParams());

	// RHICmdList.Transition(FRHITransitionInfo(RenderTargetResource->GetRenderTargetTexture(), ERHIAccess::RTV, ERHIAccess::SRVMask));
}

void UQxRenderBPLib::PostClear_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneRenderTargets& SceneRenderTargets)
{
	// Used to gather CPU profiling data for the UE4 session frontend
	// 添加给front end等用的标记
	QUICK_SCOPE_CYCLE_COUNTER(STAT_QxShaderPlugin_PixelShader);
	// Used to profile GPU activity and add metadata to be consumed by for example RenderDoc
	// 添加给renderdoc等用的标记
	SCOPED_DRAW_EVENT(RHICmdList, QxShaderPlugin_Pixel_Clear);


	//获得 当前render target的color buffer
	FTextureRHIRef rtTextureRHI = SceneRenderTargets.GetSceneColorSurface();
	FRHIRenderPassInfo RPInfo(rtTextureRHI, ERenderTargetActions::DontLoad_Store);
	TransitionRenderPassTargets(RHICmdList, RPInfo);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("ClearRT"));
	DrawClearQuad(RHICmdList, FLinearColor::Green);
	RHICmdList.EndRenderPass();
}

FSceneRenderTargetItem& UQxRenderBPLib::RequestSurface(FRHICommandListImmediate& RHICmdList)
{
	if (PooledRenderTarget.IsValid())
	{
		RHICmdList.Transition(FRHITransitionInfo(PooledRenderTarget->GetRenderTargetItem().TargetableTexture, ERHIAccess::Unknown, ERHIAccess::RTV));
		return PooledRenderTarget->GetRenderTargetItem();
	}
	FSceneRenderTargets& SceneRenderTargets = FSceneRenderTargets::Get(RHICmdList);
	const FTextureRHIRef& rtTexture = SceneRenderTargets.GetSceneColorSurface();
	
	FPooledRenderTargetDesc RenderTargetDesc =
	FPooledRenderTargetDesc::Create2DDesc(
		FIntPoint(rtTexture->GetSizeXYZ().X, rtTexture->GetSizeXYZ().Y)
		, PF_B8G8R8A8, FClearValueBinding::None, TexCreate_None,
		TexCreate_ShaderResource, false);
	if (!RenderTargetDesc.IsValid())
	{
		// useful to use the CompositingGraph dependency resolve but pass the data between nodes differently
		static FSceneRenderTargetItem Null;

		return Null;
	}

	if (!PooledRenderTarget)
	{
		GRenderTargetPool.FindFreeElement(RHICmdList, RenderTargetDesc, PooledRenderTarget, RenderTargetDesc.DebugName);
	}

	check(!PooledRenderTarget->IsFree());

	FSceneRenderTargetItem& RenderTargetItem = PooledRenderTarget->GetRenderTargetItem();
	RenderTargetItem.TargetableTexture = RenderTargetItem.ShaderResourceTexture;
	if (PooledRenderTarget.IsValid())
	{
		RHICmdList.Transition(FRHITransitionInfo(PooledRenderTarget->GetRenderTargetItem().TargetableTexture, ERHIAccess::Unknown, ERHIAccess::RTV));
		return PooledRenderTarget->GetRenderTargetItem();
	}
	return RenderTargetItem;
}

#undef LOCTEXT_NAMESPACE