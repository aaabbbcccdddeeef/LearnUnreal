// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Subsystems/EngineSubsystem.h"
#include "ZZClipperManager.generated.h"

class AZZClippingVolume;

// #TODO 这里需不需要继承render resource
class FZZCliperRenderData
{
public:
    FStructuredBufferRHIRef ClippingVolumesSB;
    FShaderResourceViewRHIRef ClippingVolumesSRV;
    
    TArray<FMatrix> ClippingVolumes;
    uint32 NumUploadedVolumes = 0; //实际上传的gpu的volume数据，应该和clippingVolumes的capacity 一致,显示的指明为了可读性
    uint32 NumClippingVolumes = 0;
    void ReInit(TArray<AZZClippingVolume*>* Array);

    void ReInit(TArray<FMatrix>& InTestMatrix, uint32 InTestNum)
    {
        ClippingVolumes = InTestMatrix;
        NumClippingVolumes = InTestNum;
        NumUploadedVolumes = ClippingVolumes.Num();

        ENQUEUE_RENDER_COMMAND(QxTestUpdateBuffer)(
          [this](FRHICommandListImmediate& RHICmdList)
          {
              ReInit_RenderThread(RHICmdList);
          }
          );
    }

    void ReInit_RenderThread(FRHICommandListImmediate& RHICmdList)
    {
        check(IsInRenderingThread());

        //#TODO buffer已经分配并且够的情况不需要重新分配
        if (ClippingVolumesSB.IsValid())
        {
            ClippingVolumesSB.SafeRelease();
        }
        if (ClippingVolumesSRV.IsValid())
        {
            ClippingVolumesSRV.SafeRelease();
        }

        // TResourceArray 是渲染资源的array，一般情况和tarray相同，UMA的情况下不同
        TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
        // ResourceArray->Reserve(RenderData->ClippingVolumes.Num());
        ResourceArray->Append(ClippingVolumes);
			
        // 预期回先用compute shader更新这个buffer，再渲染
        FRHIResourceCreateInfo ResourceCI;
			
        ResourceCI.ResourceArray = ResourceArray;
        ResourceCI.DebugName = TEXT("QxTestSB");
			
        ClippingVolumesSB = RHICreateStructuredBuffer(
            sizeof(FMatrix),
            sizeof(FMatrix) * ClippingVolumes.Num(),
            BUF_ShaderResource | BUF_Dynamic,
            ResourceCI
            );

        ClippingVolumesSRV = RHICreateShaderResourceView(
            ClippingVolumesSB
            );
    }
};

/**
 *  #TODO 这样这个类中的多线程同步的问题
 */
UCLASS()
class UZZClipperSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void OnLevelChanged();

    UFUNCTION(BlueprintCallable, Category="QxTest")
    void TestUpdateRenderData();

    // 注意，只在渲染线程调用这个方法
    inline FZZCliperRenderData* GetClipperRenderData_RenderThread() const
    {
        return  ZZClipperRenderData;
    };
private:
    // void UpdateClipperData_RenderThread(
    //     FRHICommandListImmediate& RHICmdList,
    //     TArray<AZZClippingVolume*>* ClippingVolumes);



protected:
    UPROPERTY(EditAnywhere, Category="QxTest")
    TArray<FMatrix> TestMatrix;
    
private:
    UPROPERTY()
    TArray<AZZClippingVolume*> ZZClippingVolumes;

    // 
    // 现在能想到的需要注意的问题：这个RenderData应该在渲染线程中更新
    // 这个render data数据改变时，dynamic path每帧获得更新后的数据
    // static path 可能需要主动mark dirty获得更新后的数据
    // 这个对象一旦创建就不会重新创建,只更新内容
    FZZCliperRenderData* ZZClipperRenderData = nullptr;

    FCriticalSection ClipperRenderDataCriticalSection;
};

UCLASS()
class UZZClipperManagerComponent : public UPrimitiveComponent
{
    GENERATED_BODY()
public:
    
};

UCLASS()
class AZZClipperManager : public AActor
{
    GENERATED_BODY()
public:
    AZZClipperManager();

    UPROPERTY()
    UZZClipperManagerComponent* ZZClipperManagerCom = nullptr; 
};
