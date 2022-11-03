// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZClipperCollector.h"

#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UZZClipperCollector::UZZClipperCollector()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
    
    // ...
}


// Called when the game starts
void UZZClipperCollector::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}


// Called every frame
void UZZClipperCollector::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UZZClipperCollector::CollectAndUpdateClipperTexture()
{
    if (!ClipperInfosTexture)
        return;

    
}

bool UZZClipperCollector::ValidateClipperTextureSettings() const
{
    bool bValidateCorrect = false;
    if (ClipperInfosTexture)
    {
        
    }
    return bValidateCorrect;
}

void UZZClipperCollector::ResetTextureSettings()
{
    check(ClipperInfosTexture);
    // ClipperInfosTexture->ResizeTarget()
}

