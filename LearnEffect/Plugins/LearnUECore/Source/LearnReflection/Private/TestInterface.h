// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TestInterface.generated.h"


UINTERFACE(BlueprintType)
class UMyInterface : public UInterface
{
    // GENERATED_UINTERFACE_BODY()
    GENERATED_BODY()
};

class IMyInterface
{
    GENERATED_IINTERFACE_BODY()
public:
    UFUNCTION(BlueprintImplementableEvent)
    void BPFunc() const;
};
