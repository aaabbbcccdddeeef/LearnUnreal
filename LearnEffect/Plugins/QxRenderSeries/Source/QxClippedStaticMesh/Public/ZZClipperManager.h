// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Subsystems/EngineSubsystem.h"
#include "RHI.h"
#include "RenderResource.h"
#include "UniformBuffer.h"
#include "VertexFactory.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "SceneManagement.h"
#include "ZZClipperManager.generated.h"

class UZZClipperSubsystem;
class AZZClippingVolume;

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FZZClippingVolumeParameters,)
    SHADER_PARAMETER(uint32, ZZClippingVolumeNum)
    SHADER_PARAMETER_SRV(StructuredBuffer<float4x4>, ZZClipingVolumesSB)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

DECLARE_MULTICAST_DELEGATE(FOnClippingVolumesUpdate)


class FZZClipperRenderResource : public FRenderResource
{
public:
    FZZClipperRenderResource()
        : FRenderResource(ERHIFeatureLevel::SM5)
    {
    }

    virtual void InitRHI() override;


    virtual void ReleaseRHI() override;

    void Resize(uint32 RequestedCapcity);

    uint32 GetCapcity() const;

public:

    FStructuredBufferRHIRef ClippingVolumesSB;
    FShaderResourceViewRHIRef ClippingVolumesSRV;
    uint32 ClippingVolumesNum = 0;
    

    TUniformBufferRef<FZZClippingVolumeParameters> CachedZZClipVolumeParams;

private:
    //实际上传的gpu的volume数据，应该和clippingVolumes的capacity 一致,显示的指明为了可读性
    uint32 Capcity = 1; 
};

// #TODO 这里需不需要继承render resource
// 现在来看继承FRenderResource应该是更合理的做法, 参考 FHZBOcclusionTester
// 参考 FQxDeformMeshProxy
// 参考 FHZBOcclusionTester::Submit 实现更新texture，并用texture作为参数计算
// 参考 FLidarPointCloudRenderBuffer, FLidarPointCloudOctreeNode
class FZZCliperRenderData 
{
    static constexpr int32 DefaultCapcity = 10;
public:
    FZZCliperRenderData()
    {
        // ClippingVolumes.AddDefaulted(DefaultCapcity);

        ZZClipperRenderResource = new FZZClipperRenderResource();
        BeginInitResource(ZZClipperRenderResource);
    }

    ~FZZCliperRenderData()
    {
        check(ZZClipperRenderResource);
        ZZClipperRenderResource->ReleaseResource();
        delete ZZClipperRenderResource;
        ZZClipperRenderResource = nullptr;
    }
    
public:
    void UpdateRenderData_RenderThread(FRHICommandListImmediate& RHICmdList, UZZClipperSubsystem* InZzClipperSubsystem,TArray<FMatrix>& InTestMatrix);
    
    FZZClipperRenderResource* GetZZClipperRenderResource() const
    {
        return  ZZClipperRenderResource;
    }

public:
private:
    FZZClipperRenderResource* ZZClipperRenderResource = nullptr;
    // 这个是最终上传的GPU的数据，一次全部上传
    TArray<FMatrix> ClippingVolumes;
};

/**
 *  #TODO 这样这个类中的多线程同步的问题
 */
UCLASS()
class UZZClipperSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    virtual void Tick(float DeltaTime) override;

    virtual bool IsTickableInEditor() const override final;

    virtual TStatId GetStatId() const override;

    void OnLevelChanged();

    UFUNCTION(BlueprintCallable, Category="QxTest")
    void UpdateTestData();

    TUniformBufferRef<FZZClippingVolumeParameters> GetZZClippingParams() const
    {
        return  ZZClipperRenderData->GetZZClipperRenderResource()->CachedZZClipVolumeParams;
    }

    // 注意，只在渲染线程调用这个方法
    inline FZZCliperRenderData* GetClipperRenderData_RenderThread() const
    {
        return  ZZClipperRenderData;
    };

    UFUNCTION(BlueprintCallable, Category="QxTest")
    void MarkClippingVolumesDirty();
private:

    void InitZZClipperRenderData();
    
    void UpdateZZClipperRenderData();

    TArray<FMatrix> CollectClippingVolumes() const;
public:
    // 用来通知volume 数据更新
    FOnClippingVolumesUpdate OnClippingVolumesUpdate; 

protected:
#pragma region TestMembers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QxTest")
    TArray<FMatrix> TestMatrix;

    UPROPERTY(EditAnywhere, Category="QxTest")
    uint32 NumClippingVolumes = 0;

    // clipping volume完成 encode 到这个矩阵中
    UPROPERTY(VisibleAnywhere, Category="Test")
    TArray<FMatrix> PackedZZClippingVolumes;
#pragma endregion

private:
    UPROPERTY()
    TArray<AZZClippingVolume*> ZZClippingVolumes;

    
    bool bIsClippingVolumesDirty = true;
    // 
    // 现在能想到的需要注意的问题：这个RenderData应该在渲染线程中更新
    // 这个render data数据改变时，dynamic path每帧获得更新后的数据
    // static path 可能需要主动mark dirty获得更新后的数据
    // 这个对象一旦创建就不会重新创建,只更新内容
    // 注意：这个对象是在游戏线程创建的，但由于内部有渲染资源，必须在渲染线程delete
    FZZCliperRenderData* ZZClipperRenderData = nullptr;
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
