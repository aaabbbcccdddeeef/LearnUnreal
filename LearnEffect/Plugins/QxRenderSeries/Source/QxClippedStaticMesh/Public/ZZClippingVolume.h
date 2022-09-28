// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxClippedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ZZClippingVolume.generated.h"

UCLASS(hidecategories = (Collision, Brush, Attachment, Physics, Volume, BrushBuilder), MinimalAPI)
class AZZClippingVolume : public AVolume
{
    GENERATED_BODY()

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

    AZZClippingVolume();
};