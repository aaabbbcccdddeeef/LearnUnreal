// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClippingVolume.h"

#include "Components/BrushComponent.h"

AZZClippingVolume::AZZClippingVolume()
    : bEnabled(true)
    , Mode(EZZClippingVolumeMode::ClipInside)
    , Priority(0)
{
    bColored = true;
    BrushColor.R = 0;
    BrushColor.G = 128;
    BrushColor.B = 128;
    BrushColor.A = 255;

    GetBrushComponent()->SetMobility(EComponentMobility::Movable);

    SetActorScale3D(FVector(50));
}
