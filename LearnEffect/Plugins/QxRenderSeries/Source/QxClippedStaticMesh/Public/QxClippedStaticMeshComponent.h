// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZZClippingTypes.h"
#include "Components/ActorComponent.h"
#include "QxClippedStaticMeshComponent.generated.h"


// 这组件的目的是可以添加多个 clipper的支持
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QXCLIPPEDSTATICMESH_API UQxClippedStaticMeshComponent : public UStaticMeshComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UQxClippedStaticMeshComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    virtual void OnRegister() override;

    virtual void OnUnregister() override;
public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
private:
    void GenerateClippingVolumes();

    void OnClippingVolumeBufferResize();
private:

    // 这里用一个指针比较合适，但现在先不管#TODO
    TArray<FZZPointCloudClippingVolumeParams> TestClippingVolumes;

    FDelegateHandle ClipperDelegateHandle;
};
