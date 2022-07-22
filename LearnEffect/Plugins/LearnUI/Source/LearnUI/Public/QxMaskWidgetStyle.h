// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "QxMaskWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct LEARNUI_API FQxMaskWidgetStyle2 : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FQxMaskWidgetStyle2();
	virtual ~FQxMaskWidgetStyle2();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FQxMaskWidgetStyle2& GetDefault();
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UQxMaskWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FQxMaskWidgetStyle2 WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&WidgetStyle);
	}
};
