// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClipperCollectorSubsystem.h"

#include "EngineUtils.h"
#include "ZZClipVolume.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

void UZZClipperCollectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (ClipperInfosTexture == nullptr)
    {
        UTextureRenderTarget2D* Target =
            LoadObject<UTextureRenderTarget2D>(nullptr, TEXT("/QxRenderSeries/ZZMultiClipper/RT_ClipperInfos.RT_ClipperInfos"));
        if (Target)
        {
            ClipperInfosTexture = Target;
        }
    }
    if (ClipperInfosTexture == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"Doest not find a clipper texture"));
    }

    if (MPC_Clipper == nullptr)
    {
        UMaterialParameterCollection* MPC =
            LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/QxRenderSeries/ZZMultiClipper/MPC_ZZClipper.MPC_ZZClipper"));
        if (MPC)
        {
            MPC_Clipper = MPC;
        }
    }
    if (MPC_Clipper == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"Doest not find a default materail parameter collection"));
    }

    const int32 MinClipperTexSizeX = 4;
    if (ClipperInfosTexture)
    {
        if (!ValidateClipperTextureSettings(MinClipperTexSizeX))
        {
            RegnerateTexture(MinClipperTexSizeX);
        }
    }
}

void UZZClipperCollectorSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UZZClipperCollectorSubsystem::Tick(float DeltaTime)
{
    bIsClippingVolumesDirty = true;
    if (bIsClippingVolumesDirty && ClipperInfosTexture)
    {
        CollectAndUpdateClipperTexture();
        bIsClippingVolumesDirty = false;
    }
}

bool UZZClipperCollectorSubsystem::IsTickableInEditor() const
{
    return true;
}

TStatId UZZClipperCollectorSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UZZClipperCollectorSubsystem, STATGROUP_Tickables);   
}

void UZZClipperCollectorSubsystem::CollectAndUpdateClipperTexture()
{
    if (!ClipperInfosTexture || !MPC_Clipper)
    {
        UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__" Clipper system in incorrect state"));
        return;
    }

    if (!FApp::GetGraphicsRHI().Equals(TEXT("DirectX 11")))
    {
        UE_LOG(LogTemp, Error, TEXT("Clipper system only implemented in directX 11"));
        return;
    }

    TArray<FZZClipperInfos> ClipperInfos = CollectClipperInfos();
    CachedClipperVolumeNum = ClipperInfos.Num();
    
    TArray<FLinearColor> ClipperInfosInColor = EncodeClipperInfosInColor(ClipperInfos);

     
    if (ClipperInfosInColor.Num() > (ClipperInfosTexture->SizeX * ClipperInfosTexture->SizeY))
    {
        RegnerateTexture(ClipperInfos.Num());
    }

    // UpdateClipperTexture(MoveTemp(ClipperInfosInColor));
    UpdateClipperTexAndCount(ClipperInfosInColor);
    UpdateClipperVolumeNum();
}

void UZZClipperCollectorSubsystem::MarkClipInfosDirty()
{
    bIsClippingVolumesDirty = true;
}

TArray<FZZClipperInfos> UZZClipperCollectorSubsystem::CollectClipperInfos_Implementation()
{
    TArray<FZZClipperInfos> ClipperInfoses;
    //#TODO

    UWorld* World = GetWorld();
    if (!World)
    {
        return ClipperInfoses;
    }
    FZZClipperInfos Tmp;
    for (TActorIterator<AZZClipVolume> It(World); It; ++It)
    {
        AZZClipVolume* ClipVolume = *It;
        if (ClipVolume && ClipVolume->bEnabled)
        {
            Tmp.Center = ClipVolume->GetActorLocation();
            Tmp.BoxExtent = ClipVolume->GetActorScale3D() * 100;
            Tmp.ForwardVector = ClipVolume->GetActorForwardVector();
            Tmp.UpVector = ClipVolume->GetActorUpVector();
            Tmp.Radius = ClipVolume->Radius;
            Tmp.ClipperType = ClipVolume->ClipperType;
        }
        ClipperInfoses.Add(Tmp);
        // Tmp.Center = 
    }
    return MoveTemp(ClipperInfoses);
}


void UZZClipperCollectorSubsystem::UpdateClipperVolumeNum()
{
    if (!GetWorld() || !MPC_Clipper)
    {
        return;
    }
    UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), MPC_Clipper, TEXT("ClipperVolumeNum"),
        GetClipperVolumeNum());   
}


TArray<FLinearColor> UZZClipperCollectorSubsystem::EncodeClipperInfosInColor(
    const TArray<FZZClipperInfos>& ClipperInfos)
{
    TArray<FLinearColor> EncodeColors;
    EncodeColors.Reserve(ClipperInfos.Num() * ClipperTexSizeX);
    //FZZClipperInfos 按照这样的方式encode，一个FZZClipperInfos encode成一行纹素，一行纹素有4个
    for (const FZZClipperInfos& ClipperInfo : ClipperInfos)
    {
        FLinearColor Pixel1 = FLinearColor(ClipperInfo.Center);
        // 第一个像素的a表示clipper类型
        Pixel1.A = ClipperInfo.ClipperType == EZZClipperType::Box ? 1 : 0;
        FLinearColor Pixel2 = FLinearColor(ClipperInfo.BoxExtent);
        //第二个像素的a表示radius
        Pixel2.A = ClipperInfo.Radius;
        FLinearColor Pixel3 = FLinearColor(ClipperInfo.ForwardVector);
        FLinearColor Pixel4 = FLinearColor(ClipperInfo.UpVector);
        EncodeColors.Add(Pixel1);
        EncodeColors.Add(Pixel2);
        EncodeColors.Add(Pixel3);
        EncodeColors.Add(Pixel4);
    }

    check(EncodeColors.Num() == (ClipperInfos.Num() * ClipperTexSizeX));
    return MoveTemp(EncodeColors);
}

void UZZClipperCollectorSubsystem::UpdateClipperTexture(
    const TArray<FLinearColor>& ClipperInfoColors)
{
    FTexturePlatformData** PlatformData =  ClipperInfosTexture->GetRunningPlatformData();
    if (PlatformData && *PlatformData)
    {
        FTexture2DMipMap& MipMap = (*PlatformData)->Mips[0];
        const int32 DataSize = ClipperInfoColors.Num() * sizeof(FLinearColor);
        check(DataSize <= MipMap.BulkData.GetBulkDataSize());
        void* DataPtr = MipMap.BulkData.Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(DataPtr, ClipperInfoColors.GetData(), DataSize);
        MipMap.BulkData.Unlock();
        ClipperInfosTexture->UpdateResource();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT(__FUNCTION__" Update clipperinfos texture failed"));
    }
}


/**
 * @brief 更新clipperTexture 和count, 注意count的更新会慢1帧，不过应该可以接受
 * @param ClipperInfoColors 
 */
void UZZClipperCollectorSubsystem::UpdateClipperTexAndCount(TArray<FLinearColor>& ClipperInfoColors)
{
    FTextureReferenceRHIRef TargetTexRHI = ClipperInfosTexture->TextureReference.TextureReferenceRHI;
    UZZClipperCollectorSubsystem* ClipperSystem = this;
    ENQUEUE_RENDER_COMMAND(UpdateClipperInfoTexture)(
        [ClipperSystem, TargetTexRHI, ClippeColors = MoveTemp(ClipperInfoColors)](FRHICommandListImmediate& RHICmdList)
        {
            FRHITexture* TexRef = TargetTexRHI->GetTextureReference()->GetReferencedTexture();
            FRHITexture2D* TexRHIRef = static_cast<FRHITexture2D*>(TexRef);
            
            uint32 LolStride = 0;
            char* TexDataPtr =  (char*)RHICmdList.LockTexture2D(TexRHIRef, 0, EResourceLockMode::RLM_WriteOnly, LolStride, false);

            
            if (TexRHIRef->GetFormat() == EPixelFormat::PF_FloatRGBA)
            {
                // 验证row pitch和每行数据的字节一致
                if (LolStride == sizeof(FFloat16Color) * ClipperTexSizeX)
                {
                    TArray<FFloat16Color> ConvertedColors(ClippeColors);
                    const int32 DataSize = ConvertedColors.Num() * sizeof(FFloat16Color);
                    FMemory::Memcpy(TexDataPtr, ConvertedColors.GetData(), DataSize);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("clipper texture row stride doenot match with data"));
                }
            }
            else
            {
                if (LolStride == sizeof(FLinearColor) * ClipperTexSizeX)
                {
                    const int32 DataSize = ClippeColors.Num() * sizeof(FLinearColor);
                    FMemory::Memcpy(TexDataPtr, ClippeColors.GetData(), DataSize );
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("clipper texture row stride doenot match with data"));
                }
            }
            

            RHICmdList.UnlockTexture2D(TexRHIRef, 0, false);
        }
        );
}

bool UZZClipperCollectorSubsystem::ValidateClipperTextureSettings(int32 MinTexSizeY) const
{
    bool settingValidate = ClipperInfosTexture->NeverStream;
    settingValidate &= !ClipperInfosTexture->SRGB;

    // #TODO PF_FloatRGBA应该就能满足需要，但现在纹理像素的更新上有些疑问，先用最复杂的
    bool formatValidate =  ClipperInfosTexture->GetFormat() == PF_A32B32G32R32F
         || ClipperInfosTexture->GetFormat() == PF_FloatRGBA;
    int32 BlockBytes = GPixelFormats[ ClipperInfosTexture->GetFormat()].BlockBytes;
    bool blockSizeValidate = BlockBytes == sizeof(FVector4);

    bool sizeValidate = ClipperInfosTexture->SizeY >= MinTexSizeY;
    return settingValidate && formatValidate && sizeValidate && blockSizeValidate;    
}

void UZZClipperCollectorSubsystem::RegnerateTexture(int32 MinTexSizeY)
{
    ClipperInfosTexture->NeverStream = true;
    ClipperInfosTexture->SRGB = 0;

    if (ClipperTextureFormat == ETextureRenderTargetFormat::RTF_RGBA16f)
    {
        ClipperInfosTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
    }
    else
    {
        ClipperInfosTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA32f;
    }
    if (MinTexSizeY > ClipperInfosTexture->SizeY)
    {
        // 选择大于 MinTexSizeY的最近的 power of two的数值
        uint32 TargetSizeY = FMath::RoundUpToPowerOfTwo(MinTexSizeY);
        ClipperInfosTexture->ResizeTarget(ClipperTexSizeX , TargetSizeY);
    }
    
    ClipperInfosTexture->UpdateResource();
}
