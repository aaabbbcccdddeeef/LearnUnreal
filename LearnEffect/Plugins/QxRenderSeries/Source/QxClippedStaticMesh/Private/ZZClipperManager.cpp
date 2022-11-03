// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClipperManager.h"

#include "EngineUtils.h"
#include "ZZClippingVolume.h"


TAutoConsoleVariable<int32> CVarUseTestClipMatrix(
TEXT("r.QxRender.UseTestClipMatrix"),
0,
TEXT("UseTestClipMatrix"),
ECVF_RenderThreadSafe
);


IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FZZClippingVolumeParameters, "ZZClippingVolumeParam");


void FZZClipperRenderResource::InitRHI()
{
    check(IsInRenderingThread());
    check(Capcity > 0);

    // bool IsSBNotEnogh = (ClippingVolumes.Num() * sizeof(FMatrix)) > ClippingVolumesSB->GetSize();
        
    //#TODO buffer已经分配并且够的情况不需要重新分配

    // TResourceArray 是渲染资源的array，一般情况和tarray相同，UMA的情况下不同
    // TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
    // // ResourceArray->Reserve(RenderData->ClippingVolumes.Num());
    // ResourceArray->Append(ClippingVolumes);
			
    // 预期回先用compute shader更新这个buffer，再渲染
    FRHIResourceCreateInfo ResourceCI;
			
    // ResourceCI.ResourceArray = ResourceArray;
    ResourceCI.DebugName = TEXT("QxTestSB");
			
    ClippingVolumesSB = RHICreateStructuredBuffer(
        sizeof(FMatrix),
        sizeof(FMatrix) * Capcity,
        BUF_ShaderResource | BUF_Dynamic,
        ResourceCI
    );

    ClippingVolumesSRV = RHICreateShaderResourceView(
        ClippingVolumesSB
    );

    FZZClippingVolumeParameters ZZClippingVolumeParameters;
    ZZClippingVolumeParameters.ZZClippingVolumeNum = ClippingVolumesNum;
    ZZClippingVolumeParameters.ZZClipingVolumesSB = ClippingVolumesSRV;
    // CachedZZClipVolumeParams = TUniformBuffer<FZZClippingVolumeParameters>::GetUniformBufferRef()
    CachedZZClipVolumeParams = TUniformBufferRef<FZZClippingVolumeParameters>::CreateUniformBufferImmediate(
        ZZClippingVolumeParameters, UniformBuffer_MultiFrame, EUniformBufferValidation::None);
}

void FZZClipperRenderResource::ReleaseRHI()
{
    check(IsInRenderingThread());
    if (ClippingVolumesSB.IsValid())
    {
        ClippingVolumesSB.SafeRelease();
    }
    if (ClippingVolumesSRV.IsValid())
    {
        ClippingVolumesSRV.SafeRelease();
    }
    if (CachedZZClipVolumeParams.IsValid())
    {
        CachedZZClipVolumeParams.SafeRelease();
    }
}

void FZZClipperRenderResource::Resize(uint32 RequestedCapcity)
{
    check(IsInRenderingThread());
    if (Capcity != RequestedCapcity)
    {
        ReleaseResource();
        Capcity = RequestedCapcity;
        InitResource();
    } else if (!IsInitialized())
    {
        InitResource();
    }
}

uint32 FZZClipperRenderResource::GetCapcity() const
{
    return Capcity;
}

void FZZClipperRenderResource::UpdateClipVolumeUniforms()
{
    FZZClippingVolumeParameters ZZClippingVolumeParameters;
    ZZClippingVolumeParameters.ZZClippingVolumeNum = ClippingVolumesNum;
    ZZClippingVolumeParameters.ZZClipingVolumesSB = ClippingVolumesSRV;
    CachedZZClipVolumeParams.UpdateUniformBufferImmediate(ZZClippingVolumeParameters);
}

void FZZCliperRenderData::UpdateRenderData_RenderThread(
    FRHICommandListImmediate& RHICmdList,
    UZZClipperSubsystem* InZzClipperSubsystem,
    TArray<FMatrix>& InTestMatrix)
{
    ClippingVolumes = MoveTemp(InTestMatrix);
    // NumClippingVolumes = InTestNum;
    // NumUploadedVolumes = ClippingVolumes.Num();

    check(IsInRenderingThread())
        
    if (static_cast<uint32>(ClippingVolumes.Num()) > ZZClipperRenderResource->GetCapcity())
    {
        ZZClipperRenderResource->Resize(ClippingVolumes.Num() + 1); // 一次多请求10个capcity
        
        
        // 下面的做法不对，
        // 当前帧中，引用的buffer完成是否
        // 由于渲染线程比游戏线程慢一帧,现在通知clipper，clipper再通知其他markdirty，当前帧中其他引用了buffer的东西可能已经回收
        AsyncTask(ENamedThreads::GameThread, [InZzClipperSubsystem]()
           {
               InZzClipperSubsystem->OnClippingVolumesUpdate.Broadcast();
           });
    }

    ZZClipperRenderResource->ClippingVolumesNum = ClippingVolumes.Num();
    if (ClippingVolumes.Num() > 0)
    {
        const uint32 UpdateSize = sizeof(FMatrix) * ClippingVolumes.Num();
        uint8* DataPtr =  (uint8*)RHILockStructuredBuffer(
            ZZClipperRenderResource->ClippingVolumesSB,
            0,
            UpdateSize,
            RLM_WriteOnly);
        // FMemory::Memzero(DataPtr, UpdateSize);
        FMemory::Memcpy(DataPtr, ClippingVolumes.GetData(), UpdateSize);
        RHIUnlockStructuredBuffer(ZZClipperRenderResource->ClippingVolumesSB);
    }
   
    ZZClipperRenderResource->UpdateClipVolumeUniforms();
}


void UZZClipperSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // GetWorld()->OnLevelsChanged().AddUObject(this, &UZZClipperSubsystem::OnLevelChanged);

    // TestUpdateRenderData();
    InitZZClipperRenderData();
    UpdateTestData();
}

void UZZClipperSubsystem::Deinitialize()
{
    // 这里参考 FScene::RemovePrimitive，  FScene::RemovePrimitiveSceneInfo_RenderThread对 FPrimitiveSceneProxy的处理
    // 在渲染线程delete这个对象
    ENQUEUE_RENDER_COMMAND(DeleteZZRenderData)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            if (ZZClipperRenderData)
            {
                delete ZZClipperRenderData;
                ZZClipperRenderData = nullptr;
            }
        }
        );
    
    ZZClippingVolumes.Reset();
    Super::Deinitialize();
}

void UZZClipperSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsClippingVolumesDirty)
    {
        UpdateZZClipperRenderData();
    }
}

bool UZZClipperSubsystem::IsTickableInEditor() const
{
    return true;
}

TStatId UZZClipperSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UZZClipperSubsystem, STATGROUP_Tickables);   
    // return Super::GetStatId();
}

void UZZClipperSubsystem::BeginDestroy()
{
    Super::BeginDestroy();
    DetachFence.BeginFence();
}

bool UZZClipperSubsystem::IsReadyForFinishDestroy()
{
    // Don't allow the primitive component to the purged until its pending scene detachments have completed.
    return Super::IsReadyForFinishDestroy() && DetachFence.IsFenceComplete();
}

void UZZClipperSubsystem::OnLevelChanged()
{
    ZZClippingVolumes.Reset();
    UWorld* World = GetWorld();
    for (TActorIterator<AZZClippingVolume2> It(World); It; ++It)
    {
        AZZClippingVolume2* ClipVolume = *It;
        if (ClipVolume)
        {
            ZZClippingVolumes.Add(ClipVolume);
        }
    }

    TArray<AZZClippingVolume2*>* ZZClippingVolumesPtr = &ZZClippingVolumes;

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

void UZZClipperSubsystem::UpdateTestData()
{
    if (TestMatrix.Num() == 0)
    {
        TestMatrix.AddUninitialized(10);
        for (int i = 0; i < 10; ++i)
        {
            FMatrix ltestMatrix = FMatrix::Identity * i;
            TestMatrix[i] = ltestMatrix;
        }
    }
    // ZZClipperRenderData->ReInit(TestMatrix, 10);
    // TArray<FMatrix> TmpMatrix = TestMatrix;
    // FZZCliperRenderData* RenderData = ZZClipperRenderData;
    // uint32 TmpNumClippingVolumes = NumClippingVolumes;
}

void UZZClipperSubsystem::MarkClippingVolumesDirty()
{
    bIsClippingVolumesDirty = true;
}

void UZZClipperSubsystem::InitZZClipperRenderData()
{
    ZZClipperRenderData = new FZZCliperRenderData();
}

void UZZClipperSubsystem::UpdateZZClipperRenderData()
{
    TArray<FMatrix> TmpMatrix;
    bool bUseTestMatrix = CVarUseTestClipMatrix.GetValueOnAnyThread() != 0;
    if (bUseTestMatrix)
    {
        TmpMatrix = TestMatrix;
    }
    else
    {
        TmpMatrix = CollectClippingVolumes();
    }
    if (TmpMatrix.Num() <= 0)
    {
        return;
    }
    
    FZZCliperRenderData* RenderData = ZZClipperRenderData;
    uint32 TmpNumClippingVolumes = NumClippingVolumes;
    ENQUEUE_RENDER_COMMAND(QxTestUpdateBuffer)(
          [this, RenderData, ClippingMatrix = MoveTemp(TmpMatrix), TmpNumClippingVolumes](FRHICommandListImmediate& RHICmdList)
          {
              TArray<FMatrix>& Matrices = const_cast<TArray<FMatrix>&>(ClippingMatrix);
              RenderData->UpdateRenderData_RenderThread(
                  RHICmdList,this, Matrices);
          }
          );
    
    // OnClippingVolumesUpdate.Broadcast();
    // 严格来说不应该这么做，测试
    // 方法1 ：
    // FlushRenderingCommands();
    // 方法2：
    FRenderCommandFence Fence;
    Fence.BeginFence();
    Fence.Wait();
    
    bIsClippingVolumesDirty = false;
}

TArray<FMatrix> UZZClipperSubsystem::CollectClippingVolumes() const
{
    TArray<FMatrix> ClippVolumeInfos;
    UWorld* World = GetWorld();
    for (TActorIterator<AZZClippingVolume2> It(World); It; ++It)
    {
        AZZClippingVolume2* ClippingVolume = *It;
        if (ClippingVolume->bEnabled)
        {
            const FVector Extent = ClippingVolume->GetActorScale3D() * 100;
            EZZClippingVolumeMode Mode = ClippingVolume->Mode;
            FMatrix PackedShaderData = FMatrix( FPlane(ClippingVolume->GetActorLocation(), Mode == EZZClippingVolumeMode::ClipInside),
                                        FPlane(ClippingVolume->GetActorForwardVector(), Extent.X),
                                        FPlane(ClippingVolume->GetActorRightVector(), Extent.Y),
                                        FPlane(ClippingVolume->GetActorUpVector(), Extent.Z));
            ClippVolumeInfos.Add(PackedShaderData);
        }
    }
    return MoveTemp(ClippVolumeInfos);
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
