// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "UObject/Object.h"
#include "QxUserWidget.generated.h"

/**
 *  这里用abstract 表示不希望直接使用这个类，而用bp 实现这个类
 */
UCLASS(Abstract)
class LEARNUI_API UQxUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 对ui 来说这个比ctor 更有用
	virtual void NativeConstruct() override;

public:

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* TitleLabe;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UImage* IconImage;
};


UCLASS()
class UQxOverlay : public UOverlay
{
	GENERATED_BODY()

public:
	virtual const FText GetPaletteCategory() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
	virtual TSharedRef<SWidget> RebuildDesignWidget(TSharedRef<SWidget> Content) override;
public:

	
};