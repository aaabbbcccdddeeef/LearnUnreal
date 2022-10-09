// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxMultithreadTest.generated.h"

UCLASS()
class LEARNCORE_API AQxMultithreadTest : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AQxMultithreadTest();

    UFUNCTION(BlueprintCallable, Category="QxTest")
    void PrintTestData();

    void TestDived(TArray<float>& InArray);

    UFUNCTION(BlueprintCallable, Category="QxTest")
    void Test();
    
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
private:
    void GenerateTestData1();
private:

    TArray<float> TestData1;

    float Test5Sum = 0;
    FCriticalSection TestMutex;
};
