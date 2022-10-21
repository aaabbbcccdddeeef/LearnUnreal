// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxOctreeActor.generated.h"

UCLASS()
class QXACCELERATE_API AQxOctreeActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AQxOctreeActor();
    AQxOctreeActor(const FVector& Origin, const FVector& Extent);

    struct QxBounds
    {
        FVector Origin;
        FVector Extent;
    };

    // FBoxCenterAndExtent
    QxBounds QxBoundingBox;

    int32 MinNodeExtent = 0;
    int32 MaxNodeExtent = 0;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    void SetQxBoundingBox(const FVector& Origin, const FVector& Extent);

    UFUNCTION(BlueprintPure, Category="QxSpacialDataStructure")
    FVector GetQxBoundingBoxOrigin();

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    void SetParent();

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    void Init(const FVector& Origin, const FVector& Extent);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    void DrawNodes();

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    void BuildTree();
};
