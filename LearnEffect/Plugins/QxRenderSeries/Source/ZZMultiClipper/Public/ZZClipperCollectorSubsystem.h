// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZZClipperTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ZZClipperCollectorSubsystem.generated.h"




/**
 * 
 */
UCLASS()
class ZZMULTICLIPPER_API UZZClipperCollectorSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    virtual void Tick(float DeltaTime) override;

    virtual bool IsTickableInEditor() const override final;

    virtual TStatId GetStatId() const override;

    void CollectAndUpdateClipperTexture();

    UFUNCTION(BlueprintCallable, Category="ZZClippers")
    void MarkClipInfosDirty();
protected:
    UFUNCTION(BlueprintNativeEvent, Category="ZZClippers")
    TArray<FZZClipperInfos> CollectClipperInfos();

    TArray<FZZClipperInfos> CollectClipperInfos_Implementation();


    
private:
    static TArray<FLinearColor> EncodeClipperInfosInColor(const TArray<FZZClipperInfos>& ClipperInfos);

    
    void UpdateClipperTexture(const TArray<FLinearColor>& ClipperInfoColors);

    void UpdateClipperTexAndCount(TArray<FLinearColor>& ClipperInfoColors);
private:
    // ClipperTexture的宽是一定的
    static constexpr int32 ClipperTexSizeX = 4;
    
    // 验证render target的格式设置
    /**
     * @brief 
     * @param MinTexSizeY texture的最小宽度,height固定是ClipperTexSizeY
     * @return 
     */
    bool ValidateClipperTextureSettings(int32 MinTexSizeY) const;

    // 更新render target设置以符合需要
    void RegnerateTexture(int32 MinTexSizeY);

    int32 GetClipperVolumeNum() const
    {
        return CachedClipperVolumeNum;
    };

private:
    void UpdateClipperVolumeNum();
    
protected:
    UPROPERTY(EditAnywhere, Category="ZZClippers")
    UTextureRenderTarget2D* ClipperInfosTexture = nullptr;
    // UTexture2D* ClipperInfosTexture = nullptr;

    UPROPERTY(EditAnywhere, Category="ZZClippers")
    UMaterialParameterCollection* MPC_Clipper = nullptr;

    UPROPERTY(EditAnywhere, Category="ZZClippers")
    ETextureRenderTargetFormat  ClipperTextureFormat = ETextureRenderTargetFormat::RTF_RGBA32f;
private:
        
    int32 CachedClipperVolumeNum = 0;
    bool bIsClippingVolumesDirty = false;
};


