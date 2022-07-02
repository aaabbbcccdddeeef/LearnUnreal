// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMultiOccluder.h"

#include "QxRenderBPLib.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialParameterCollectionInstance.h"


const int32 width = 4;
const int32 height = 128;

// Sets default values for this component's properties
UQxMultiOccluder::UQxMultiOccluder()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UQxMultiOccluder::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UQxMultiOccluder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UQxMultiOccluder::BeginDestroy()
{
	Super::BeginDestroy();
	
}


// Called every frame
void UQxMultiOccluder::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UQxMultiOccluder::CollectOccluderInfos(TArray<FBoxOccluderInfo>& OutOccluderInfos, int32& OutOccluderNum)
{
	OutOccluderInfos.Empty();
	int OutCcluderNum;
	TArray<FBoxOccluderInfo> tmp = CollectOccluderInfos_BP(OutCcluderNum);
	OutOccluderInfos.Append(tmp);
	// FBoxOccluderInfo testData;
	// testData.Center = FVector(0, 500, 0);
	// testData.BoxExtent = FVector(0, 500, 0);
	// testData.ForwardVector = FVector(0, 500, 0);
	// testData.UpVector = FVector(0, 500, 0);
	//
	// for (int i = 0; i < 4 * 64; ++i)
	// {
	// 	OutOccluderInfos.Add(testData);
	// }
}

void UpdateOccluderTexture_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	UTextureRenderTarget2D* InOccluderTexture,
	int32 InOccluderNum,
	const TArray<FBoxOccluderInfo>& InOccluderInfos)
{
	check(IsInRenderingThread());
	if (InOccluderTexture == nullptr)
	{
		return;
	}

	// UWorld* world = GetWorld();
	// ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();

	FTextureReferenceRHIRef targetTexRHI = InOccluderTexture->TextureReference.TextureReferenceRHI;
	FRHITexture* texRef = targetTexRHI->GetTextureReference()->GetReferencedTexture();
	FRHITexture2D* texRef2D = (FRHITexture2D*)(texRef);

	TArray<FColor> bitmap;
	uint32 LolStride = 0;
	char* textureDataPtr = (char*)RHICmdList.LockTexture2D(texRef2D, 0,
		EResourceLockMode::RLM_WriteOnly, LolStride, false);
	
	TArray<FLinearColor> ColorData;
	ColorData.AddUninitialized(width * height);
	for (int32 i = 0; i < width * height; ++i)
	{
		float test = FMath::RandRange(20, 90);
		ColorData[i] = FLinearColor(i, test, i, 1);
	}

	FMemory::Memcpy(textureDataPtr, ColorData.GetData(),
		ColorData.Num() * sizeof(FLinearColor));

	RHICmdList.UnlockTexture2D(texRef2D, 0, false);
}

void UQxMultiOccluder::CollectOccluderAndUpdateTexture()
{
	if (!OccluderTexture)
	{
		return;
	}
	const int32 TextureWidth = 4;
	// UTextureRenderTarget2D* test;
	if (OccluderTexture->GetSizeX() != TextureWidth)
	{
		return;
	}

#pragma region FormatVolidate
	bool bTextureValidateResult = true;
	EPixelFormat PixelFormat = OccluderTexture->GetPixelFormat();
	bTextureValidateResult &= (EPixelFormat::PF_A32B32G32R32F == PixelFormat);
	int32 BlockBytes = GPixelFormats[PixelFormat].BlockBytes;
	check(BlockBytes == sizeof(FVector4));

	
	bTextureValidateResult &= OccluderTexture->NeverStream == true;
	 bTextureValidateResult &= OccluderTexture->SRGB == 0;
	bTextureValidateResult &= OccluderTexture->LODGroup == TextureGroup::TEXTUREGROUP_Pixels2D;
	check(bTextureValidateResult);
#pragma endregion
	
	TArray<FBoxOccluderInfo> BoxOccluderInfos ;
	int32 OccluderNum = 0;
	CollectOccluderInfos(BoxOccluderInfos, OccluderNum);


#pragma region ConvertInfoDataToFColorArray
	TArray<FLinearColor> ColorData;
	// for (int32 i = 0; i < BoxOccluderInfos.Num(); ++i)
	// {
	// 	const FBoxOccluderInfo& OccluderInfo = BoxOccluderInfos[i];
	// 	ColorData[i] = FLinearColor(OccluderInfo.Center);
	// 	ColorData[i + 1] = FLinearColor(OccluderInfo.BoxExtent);
	// 	ColorData[i + 2] = FLinearColor(OccluderInfo.ForwardVector);
	// 	ColorData[i + 3] = FLinearColor(OccluderInfo.UpVector);
	// }
	// for (int32 i = 0; i < BoxOccluderInfos.Num() * TextureWidth; ++i)
	// {
	// 	float test = FMath::RandRange(20, 90);
	// 	ColorData[i] = FLinearColor(i, test, i, i);
	// }
#pragma endregion
	ColorData.AddUninitialized(width * height);
	check(width * height == BoxOccluderInfos.Num() * 4);
	for (int32 i = 0; i < BoxOccluderInfos.Num(); ++i)
	{
		// float test = FMath::RandRange(20, 90);
		// ColorData[i] = FLinearColor(i, test, i, 1);
		const FBoxOccluderInfo& OccluderInfo = BoxOccluderInfos[i];
		ColorData[i] = FLinearColor(OccluderInfo.Center);
		ColorData[i + 1] = FLinearColor(OccluderInfo.BoxExtent);
		ColorData[i + 2] = FLinearColor(OccluderInfo.ForwardVector);
		ColorData[i + 3] = FLinearColor(OccluderInfo.UpVector);
	}
	
	// 第一种实现，游戏线程中更新，后续交给UE4
	FTexture2DMipMap& MipMap = OccluderTexture->PlatformData->Mips[0];

	int32 tmp = (ColorData.Num() * sizeof(FLinearColor)) ;
	int32 tmp2 = MipMap.BulkData.GetBulkDataSize();
	check(tmp <= tmp2);
	void* Data = MipMap.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, ColorData.GetData(),
		ColorData.Num() * sizeof(FLinearColor));
	MipMap.BulkData.Unlock();
	OccluderTexture->UpdateResource();
	
	// 第二种实现,在渲染线程中更新
#pragma region UpdateTextureInRenderThread
	// UTexture2D* LoalOccluderTexture = OccluderTexture;
	// ENQUEUE_RENDER_COMMAND(UpdateOccluderTexture)(
	// 	[LoalOccluderTexture, OccluderNum, TmpOccluderInfo = MoveTemp(BoxOccluderInfos)](FRHICommandListImmediate& RHICmdList)
	// 	{
	// 		UpdateOccluderTexture_RenderThread(LoalOccluderTexture, OccluderNum, TmpOccluderInfo);
	// 	}
	// );
#pragma endregion
	
}

void UQxMultiOccluder::CollectOccluderAndUpdateRT()
{
// 	if (!OccluderRT)
// 	{
// 		return;
// 	}
// 	const int32 TextureWidth = 4;
// 	// UTextureRenderTarget2D* test;
// 	if (OccluderRT->SizeX != TextureWidth)
// 	{
// 		return;
// 	}
//
// #pragma region FormatVolidate
// 	bool bTextureValidateResult = true;
// 	// EPixelFormat PixelFormat = OccluderRT->GetPixelFormatEnum();
// 	EPixelFormat PixelFormat = OccluderRT->TextureReference.TextureReferenceRHI->GetFormat();
// 	bTextureValidateResult &= (EPixelFormat::PF_A32B32G32R32F == PixelFormat);
// 	int32 BlockBytes = GPixelFormats[PixelFormat].BlockBytes;
// 	check(BlockBytes == sizeof(FVector4));
// 	
// 	bTextureValidateResult &= OccluderTexture->NeverStream == true;
// 	bTextureValidateResult &= OccluderTexture->SRGB == 0;
// 	bTextureValidateResult &= OccluderTexture->LODGroup == TextureGroup::TEXTUREGROUP_Pixels2D;
// 	check(bTextureValidateResult);
// #pragma endregion
// 	
// 	TArray<FBoxOccluderInfo> BoxOccluderInfos ;
// 	int32 OccluderNum = 0;
// 	CollectOccluderInfos(BoxOccluderInfos, OccluderNum);
//
//
// #pragma region ConvertInfoDataToFColorArray
// 	TArray<FLinearColor> ColorData;
// 	#pragma endregion
// 	ColorData.AddUninitialized(width * height);
// 	for (int32 i = 0; i < width * height; ++i)
// 	{
// 		float test = FMath::RandRange(20, 90);
// 		ColorData[i] = FLinearColor(i, test, i, 1);
// 	}
//
// 	UTextureRenderTarget2D* LoalOccluderTexture = OccluderRT;
// 	// UWorld* world = GetWorld();
// 	// ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();
// 	
// 	ENQUEUE_RENDER_COMMAND(UpdateOccluderTexture)(
// 		[LoalOccluderTexture, OccluderNum, TmpOccluderInfo = MoveTemp(BoxOccluderInfos)](FRHICommandListImmediate& RHICmdList)
// 		{
// 			UpdateOccluderTexture_RenderThread(RHICmdList, LoalOccluderTexture, OccluderNum, TmpOccluderInfo);
// 		}
// 	);
}



void UQxMultiOccluder::CreateTexture()
{
	
	OccluderTexture = UTexture2D::CreateTransient(width, height, PF_A32B32G32R32F);
	check(OccluderTexture);
	OccluderTexture->NeverStream = true;
	OccluderTexture->SRGB = 0;
	OccluderTexture->LODGroup = TextureGroup::TEXTUREGROUP_Pixels2D;

	TArray<FLinearColor> ColorData;
	ColorData.AddUninitialized(width * height);
	for (int32 i = 0; i < width * height; ++i)
	{
		ColorData[i] = FLinearColor(0, 0, 0, 0);
	}
#pragma endregion
	
	// 第一种实现，游戏线程中更新，后续交给UE4
	FTexture2DMipMap& MipMap = OccluderTexture->PlatformData->Mips[0];
	void* Data = MipMap.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, ColorData.GetData(),
		ColorData.Num() * sizeof(FLinearColor));
	MipMap.BulkData.Unlock();
	OccluderTexture->UpdateResource();
}

void UQxMultiOccluder::ReformatOccluderTexture()
{
	if (!OccluderTexture)
	{
		return;
	}
	UQxRenderBPLib::SetTextureForamt(OccluderTexture, OccluderTexture->GetSizeX(), OccluderTexture->GetSizeY());
}

