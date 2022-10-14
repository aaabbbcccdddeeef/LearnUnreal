// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxClippedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ZZClippingVolume.generated.h"

UCLASS(hidecategories = (Collision, Attachment, Physics), Blueprintable, MinimalAPI)
class AZZClippingVolume : public AVolume
{
    GENERATED_BODY()

public:
    AZZClippingVolume();

    virtual void PostRegisterAllComponents() override;

private:
    void OnTransformUpdated(USceneComponent* RootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clipping Volume")
    bool bEnabled;

    /** Affects how this volume affects points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clipping Volume")
    EZZClippingVolumeMode Mode;

    /**
     * Determines the processing order of the nodes, in case they overlap.
     * Higher values take priority over lower ones.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clipping Volume")
    int32 Priority;

private:
    bool bIsTransformUpdated = true;
};