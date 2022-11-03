// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClippingVolume.h"

#include "ZZClipperManager.h"
#include "Components/BrushComponent.h"

AZZClippingVolume2::AZZClippingVolume2()
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

void AZZClippingVolume2::PostRegisterAllComponents()
{
    Super::PostInitializeComponents();

    GetRootComponent()->TransformUpdated.AddUObject(this, &AZZClippingVolume2::OnTransformUpdated);
    if (UZZClipperSubsystem* ClipperSubsystem = GetWorld()->GetSubsystem<UZZClipperSubsystem>())
    {
        ClipperSubsystem->MarkClippingVolumesDirty();
    }
}

void AZZClippingVolume2::OnTransformUpdated(
    USceneComponent* InComponent,
    EUpdateTransformFlags UpdateTransformFlags,
    ETeleportType Teleport)
{
    if (UZZClipperSubsystem* ClipperSubsystem = GetWorld()->GetSubsystem<UZZClipperSubsystem>())
    {
        ClipperSubsystem->MarkClippingVolumesDirty();
    }
}
