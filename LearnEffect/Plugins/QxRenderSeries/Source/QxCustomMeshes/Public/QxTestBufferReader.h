// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QxTestBufferReader.generated.h"


class FQxTestRenderData
{
public:

    void ReInit(const TArray<FMatrix>& InMatrixs, uint32 InTestNum = 0);

    void ReInit_RenderThread(FRHICommandListImmediate& RHICmdList);
    
    FStructuredBufferRHIRef TestSB;
    FShaderResourceViewRHIRef TestSRV;

    TArray<FMatrix> TestMatrixs;
    uint32 TestNum = 0;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QXCUSTOMMESHES_API UQxTestBufferReader : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UQxTestBufferReader();

    void UpdateTestRenderDataAndBuffer();

    FQxTestRenderData* GetTestRenderData() const
    {
        return TestRenderData;
    }
public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    
protected:

    UPROPERTY(EditAnywhere, Category="QxTest")
    TArray<FMatrix> TestMatrix;

    UPROPERTY(EditAnywhere, Category="QxTest")
    uint32 TestNum;
private:

    FQxTestRenderData* TestRenderData;
};
