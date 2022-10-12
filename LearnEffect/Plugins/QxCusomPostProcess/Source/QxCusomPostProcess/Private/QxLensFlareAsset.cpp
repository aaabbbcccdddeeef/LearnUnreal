// Fill out your copyright notice in the Description page of Project Settings.


#include "QxLensFlareAsset.h"

#include "QxPostprocessSubsystem.h"

#if WITH_EDITOR
void UQxBloomFlareAsset::PreEditChange(FProperty* PropertyAboutToChange)
{
    if (GEngine->GetEngineSubsystem<UQxPostprocessSubsystem>())
    {
        GEngine->GetEngineSubsystem<UQxPostprocessSubsystem>()->PostAssetMutex.Lock();
    }
    Super::PreEditChange(PropertyAboutToChange);
}

void UQxBloomFlareAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (GEngine->GetEngineSubsystem<UQxPostprocessSubsystem>())
    {
        GEngine->GetEngineSubsystem<UQxPostprocessSubsystem>()->PostAssetMutex.Unlock();
    }
}
#endif
