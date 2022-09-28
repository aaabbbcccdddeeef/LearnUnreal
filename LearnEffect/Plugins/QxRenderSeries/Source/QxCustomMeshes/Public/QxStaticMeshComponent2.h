// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QxStaticMeshComponent2.generated.h"




// 因为这个和已经存在的2pass的static mesh component重名了，暂时想不起合适的名字就先用的2表示
// 这个组件的主要目的是支持多个volume 
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QXCUSTOMMESHES_API UQxStaticMeshComponent2 : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UQxStaticMeshComponent2();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    
};
