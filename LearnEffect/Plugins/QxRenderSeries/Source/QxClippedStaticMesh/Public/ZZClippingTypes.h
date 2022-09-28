// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EZZClippingVolumeMode : uint8
{
    /** This will clip all points inside the volume */
    ClipInside,
    /** This will clip all points outside of the volume */
    ClipOutside,
};


/** Used to pass clipping information for async processing, to avoid accessing UObjects in non-GT */
struct FZZPointCloudClippingVolumeParams
{
    EZZClippingVolumeMode Mode = EZZClippingVolumeMode::ClipInside;
    int32 Priority = 0;
    FBox Bounds;
    FMatrix PackedShaderData;

    FORCEINLINE bool operator<(const FZZPointCloudClippingVolumeParams& O) const
    {
        return (Priority < O.Priority) || (Priority == O.Priority && Mode > O.Mode);
    }

    FZZPointCloudClippingVolumeParams(const class ALidarClippingVolume* ClippingVolume);
    FZZPointCloudClippingVolumeParams() {  }
};