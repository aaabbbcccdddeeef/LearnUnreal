// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxMaskWidgetStyle.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_RetVal_TwoParams(FReply, FQxMaskOnClicked, const int32&, const bool& );

/**
 * 
 */
class LEARNUI_API SQxMaskWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SQxMaskWidget)
		: _Style(&FCoreStyle::Get().GetWidgetStyle<FQxMaskWidgetStyle2>("QxMaskWidget2"))
		, _BgColorAndOpacity(FLinearColor::White)
		{
			_Visibility = EVisibility::Visible;
		}

		SLATE_STYLE_ARGUMENT(FQxMaskWidgetStyle2, Style);

		SLATE_ATTRIBUTE(FSlateColor, BgColorAndOpacity);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
		const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	virtual bool IsInteractable() const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& GestureEvent) override;

	
public:
	bool bIsMaskeUpdated = true;
	
protected:
	FQxMaskOnClicked OnClicked;
	
	TAttribute<FSlateColor> BgColorAndOpacity;

	const FSlateBrush* BackgroundImage;
private:
	FQxMaskWidgetStyle2* Style;
};
