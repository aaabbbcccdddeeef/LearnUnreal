// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClipVolume.h"

#include "ZZClipperCollectorSubsystem.h"
#include "Components/BrushComponent.h"

// Sets default values
AZZClipVolume::AZZClipVolume()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // bColored = true;
    // BrushColor.R = 0;
    // BrushColor.G = 128;
    // BrushColor.B = 128;
    // BrushColor.A = 255;

    // GetBrushComponent()->SetMobility(EComponentMobility::Movable);

    SetActorScale3D(FVector(50));
}

void AZZClipVolume::PostRegisterAllComponents()
{
    Super::PostRegisterAllComponents();
    GetRootComponent()->TransformUpdated.AddUObject(this, &AZZClipVolume::OnTransformUpdated);
    if (UZZClipperCollectorSubsystem* ClipperSubsystem = GetWorld()->GetSubsystem<UZZClipperCollectorSubsystem>())
    {
        ClipperSubsystem->MarkClipInfosDirty();
    }
}

EDataValidationResult AZZClipVolume::IsDataValid(TArray<FText>& ValidationErrors)
{
    return Super::IsDataValid(ValidationErrors);
}

void AZZClipVolume::OnTransformUpdated(USceneComponent* InComponent, EUpdateTransformFlags UpdateTransformFlags,
                                        ETeleportType Teleport)
{
    if (UZZClipperCollectorSubsystem* ClipperSubsystem = GetWorld()->GetSubsystem<UZZClipperCollectorSubsystem>())
    {
        ClipperSubsystem->MarkClipInfosDirty();
    }
}

