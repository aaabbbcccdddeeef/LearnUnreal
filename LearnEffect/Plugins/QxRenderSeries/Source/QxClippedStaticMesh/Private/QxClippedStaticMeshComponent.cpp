// Fill out your copyright notice in the Description page of Project Settings.


#include "QxClippedStaticMeshComponent.h"


class FZZStaticMeshSceneProxy : public FStaticMeshSceneProxy
{
public:
    FZZStaticMeshSceneProxy(UQxClippedStaticMeshComponent* Component,
        bool bFoolLODsShareStaticLighting) :FStaticMeshSceneProxy(Component, bFoolLODsShareStaticLighting)
    {
        ComponentPtr = Component;
    };

    // void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;

private:
    UPROPERTY()
    const UStaticMeshComponent* ComponentPtr;

    // 这两个buffer 理论上来说可以引用这个proxy外部的，但现在先每个都创建
    FStructuredBufferRHIRef ZZClippingVolumesSB;

    FShaderResourceViewRHIRef ZZClippingVolumesSRV;
};


// Sets default values for this component's properties
UQxClippedStaticMeshComponent::UQxClippedStaticMeshComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UQxClippedStaticMeshComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}


// Called every frame
void UQxClippedStaticMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

FPrimitiveSceneProxy* UQxClippedStaticMeshComponent::CreateSceneProxy()
{
    if (GetStaticMesh() == nullptr || GetStaticMesh()->GetRenderData() == nullptr)
    {
        return nullptr;
    }

    const FStaticMeshLODResourcesArray& LODResources = GetStaticMesh()->GetRenderData()->LODResources;
    if (LODResources.Num() == 0 || LODResources[FMath::Clamp<int32>(GetStaticMesh()->GetMinLOD().Default, 0, LODResources.Num() - 1)].VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() == 0)
    {
        return nullptr;
    }
    LLM_SCOPE(ELLMTag::StaticMesh);

    //FPrimitiveSceneProxy* Proxy = ::new FStaticMeshSceneProxy(this, false);
    FPrimitiveSceneProxy* proxy = ::new FZZStaticMeshSceneProxy(this, false);
#if STATICMESH_ENABLE_DEBUG_RENDERING
    SendRenderDebugPhysics(proxy);
#endif
    return proxy;
}

void UQxClippedStaticMeshComponent::GenerateClippingVolumes()
{
    FZZPointCloudClippingVolumeParams TestParam;
    
    TestClippingVolumes.Reset();

    for (int32 i = 0; i < (16); ++i)
    {
        FZZPointCloudClippingVolumeParams tmpParam;

        FVector Extent = FVector::OneVector * 100;
        tmpParam.PackedShaderData = FMatrix(
            FPlane(FVector::ZeroVector, 1),
            FPlane(FVector::ForwardVector, Extent.X),
            FPlane(FVector::RightVector, Extent.Y),
            FPlane(FVector::UpVector, Extent.Z)
            );
        TestClippingVolumes.Add(tmpParam);
    }
}

