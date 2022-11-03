// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZZClipperTypes.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "ZZClipVolume.generated.h"

UCLASS(Blueprintable)
class ZZMULTICLIPPER_API AZZClipVolume : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AZZClipVolume();

    virtual void PostRegisterAllComponents() override;

    virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
private:
    void OnTransformUpdated(USceneComponent* InComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZZClipper")
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZZClipper")
    EZZClipperType ClipperType = EZZClipperType::Box;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZZClipper")
    float Radius = 0.f;
protected:
public:
};
