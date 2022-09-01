// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxMaskWidgetStyle.h"
#include "QxMaskWidgetStyle.h"
#include "Components/Widget.h"
#include "UObject/Object.h"
#include "QxMaskWidget.generated.h"

class SQxMaskWidget;
class SQxTestWidget;
/**
 * 
 */
UCLASS()
class LEARNUI_API UQxMaskWidget : public UWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QxUI")
	FQxMaskWidgetStyle2 QxWidgetStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QxUI")
	FLinearColor BgColorAndOpacity;

	// 可绑定的委托
	UPROPERTY()
	FGetLinearColor BgColorAndOpacityDelegate;

public:
	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetBgColorAndOpacity(FLinearColor InColorAndOpacity);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetBgOpacity(float InOpacity);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetBgImage(UTexture2D* Tex, bool bMatchSize = false);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetMaskMaterial(UMaterialInterface* InMaterial);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetBrushTintColor(FSlateColor TintColor);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void SetMaskImage(const int32& ClipIndex, UTexture2D* Tex);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void SetMaskPos(const int32& ClipIndex, const FVector2D& Pos);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void SetMaskPosXY(const int32& ClipIndex, const float& PosX, const float& PosY);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void SetMaskSize(const int32& ClipIndex, const FVector2D& Size);
	
	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void SetMaskSizeXY(const int32& ClipIndex, const float& SizeX, const float& SizeY);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void SetMaskPosSize(const int32& ClipIndex, const FVector4& PosSize);
	
	UFUNCTION(BlueprintCallable, Category="MaskClip")
	void EnableMaskClickClip(const int32& ClipIndex,bool Enable);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	int32 AddMaskClickClip(const FVector2D& Position, const FVector2D& Size, UTexture2D* Mask = nullptr);

	UFUNCTION(BlueprintCallable, Category="MaskClip")
	bool RemoveMaskClip(const int32& ClipIndex);

public:
	// Rebuild Widget会调用这个
	virtual void SynchronizeProperties() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	TSharedPtr<SQxMaskWidget> QxMask;

	// 实现property binding 组件之一
	PROPERTY_BINDING_IMPLEMENTATION(FSlateColor, BgColorAndOpacity);
};
