// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClipperManager.h"

#include "EngineUtils.h"
#include "ZZClippingVolume.h"

void FZZCliperRenderData::ReInit(TArray<AZZClippingVolume*>* InClippingVolumes)
{
    if (static_cast<uint32>(InClippingVolumes->Num()) >NumUploadedVolumes)
    {
        // 重新创建Buffer
       
    }
}

void UZZClipperSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // GetWorld()->OnLevelsChanged().AddUObject(this, &UZZClipperSubsystem::OnLevelChanged);

    TestUpdateRenderData();
}

void UZZClipperSubsystem::Deinitialize()
{
    if (ZZClipperRenderData)
    {
        delete ZZClipperRenderData;
        ZZClipperRenderData = nullptr;
    }
    ZZClippingVolumes.Reset();
    Super::Deinitialize();
}

void UZZClipperSubsystem::OnLevelChanged()
{
    ZZClippingVolumes.Reset();
    UWorld* World = GetWorld();
    for (TActorIterator<AZZClippingVolume> It(World); It; ++It)
    {
        AZZClippingVolume* ClipVolume = *It;
        if (ClipVolume)
        {
            ZZClippingVolumes.Add(ClipVolume);
        }
    }

    TArray<AZZClippingVolume*>* ZZClippingVolumesPtr = &ZZClippingVolumes;

    int32 Test = 54;
    int32& Test2 = Test;
    // 更新渲染数据
    {
        ENQUEUE_RENDER_COMMAND(ZZUpdateClipVolume)(
            [this, ZZClippingVolumesPtr](FRHICommandListImmediate& RHICmdList)
            {
                // UpdateClipperData_RenderThread(RHICmdList, ZZClippingVolumesPtr);
            }
            );
    }
}

void UZZClipperSubsystem::TestUpdateRenderData()
{
    if (ZZClipperRenderData == nullptr)
    {
        ZZClipperRenderData = new FZZCliperRenderData();
    }

    if (TestMatrix.Num() == 0)
    {
        TestMatrix.AddUninitialized(10);
        for (int i = 0; i < 10; ++i)
        {
            FMatrix ltestMatrix = FMatrix::Identity * i;
            TestMatrix[i] = ltestMatrix;
        }
    }
    ZZClipperRenderData->ReInit(TestMatrix, 10);
}

// void UZZClipperSubsystem::UpdateClipperData_RenderThread(
//     FRHICommandListImmediate& RHICmdList,
//     TArray<AZZClippingVolume*>* InClippingVolumes)
// {
//    
// }



AZZClipperManager::AZZClipperManager()
{
    ZZClipperManagerCom = CreateDefaultSubobject<UZZClipperManagerComponent>(TEXT("ZZClipperManagerComponent"));
}
