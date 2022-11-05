// Fill out your copyright notice in the Description page of Project Settings.


#include "QxCommonUtils.h"

#include "EngineUtils.h"
#include "Components/GameFrameworkComponent.h"
#include "Engine/StaticMeshActor.h"

void UQxCommonUtils::TraverseActor(UObject* WorldContext)
{
    for (TActorIterator<AStaticMeshActor> It(WorldContext->GetWorld()); It; ++It)
    {
        AStaticMeshActor* ClippingVolume = *It;
        {
            //.....
        }
    }
}

void UQxCommonUtils::GetComponents(AActor* InActor)
{
    // 参考template<class AllocatorType>
    //void GetComponents(TArray<UActorComponent*, AllocatorType>& OutComponents
    // 1.
    TInlineComponentArray<UStaticMeshComponent*> TestComponents(InActor);
    // .....

    
    TComponentIterator<UStaticMeshComponent> TestIt(InActor);
}

