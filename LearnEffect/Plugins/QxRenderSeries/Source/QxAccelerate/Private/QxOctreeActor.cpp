// Fill out your copyright notice in the Description page of Project Settings.


#include "QxOctreeActor.h"


// Sets default values
AQxOctreeActor::AQxOctreeActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

AQxOctreeActor::AQxOctreeActor(const FVector& Origin, const FVector& Extent)
{
    Init(Origin, Extent);
}

// Called when the game starts or when spawned
void AQxOctreeActor::BeginPlay()
{
    Super::BeginPlay();
    
}

// Called every frame
void AQxOctreeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQxOctreeActor::SetQxBoundingBox(const FVector& Origin, const FVector& Extent)
{
}

FVector AQxOctreeActor::GetQxBoundingBoxOrigin()
{
    return FVector::OneVector;
}

void AQxOctreeActor::SetParent()
{
    
}

void AQxOctreeActor::Init(const FVector& Origin, const FVector& Extent)
{
    PrimaryActorTick.bCanEverTick = false;
    SetQxBoundingBox(Origin, Extent);
    MinNodeExtent = 1.f;
    MaxNodeExtent = FMath::Max3(Extent.X, Extent.Y, Extent.Z);
}

void AQxOctreeActor::DrawNodes()
{
    
}

void AQxOctreeActor::BuildTree()
{
    
}

