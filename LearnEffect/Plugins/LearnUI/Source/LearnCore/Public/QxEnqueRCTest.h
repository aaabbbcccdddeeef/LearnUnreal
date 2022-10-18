// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxEnqueRCTest.generated.h"


// 测试不使用ENQUE_RENDER_COMMAND的情况下，直接通过task graph系统添加render task到渲染线程
UCLASS(Blueprintable)
class LEARNCORE_API AQxEnqueRCTest : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AQxEnqueRCTest();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestMyEnqueRenderCommand(UTextureRenderTarget2D* InRenderTarget, FLinearColor ClearColor);

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestUseEnqueRenderCommand(UTextureRenderTarget2D* InRenderTarget, FLinearColor ClearColor);

    // UPROPERTY(EditAnywhere)
    // UTextureRenderTarget2D* TestRenderTarget = nullptr;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};
