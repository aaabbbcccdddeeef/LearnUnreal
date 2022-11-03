// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZZClipperCollector.generated.h"

//这个component 负责收集场景中的clipper,并更新clipper信息的texture
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZZMULTICLIPPER_API UZZClipperCollector : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UZZClipperCollector();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    void CollectAndUpdateClipperTexture();
private:
    // 验证render target的格式设置
    bool ValidateClipperTextureSettings() const;

    void ResetTextureSettings();
protected:

    UPROPERTY(EditAnywhere, Category="ZZClippers")
    UTextureRenderTarget2D* ClipperInfosTexture = nullptr;
};
