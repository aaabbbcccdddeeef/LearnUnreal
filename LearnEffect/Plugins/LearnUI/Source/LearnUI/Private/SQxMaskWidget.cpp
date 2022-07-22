// Fill out your copyright notice in the Description page of Project Settings.


#include "SQxMaskWidget.h"

#include <stdexcept>

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQxMaskWidget::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
	check(InArgs._Style);

	BgColorAndOpacity = InArgs._BgColorAndOpacity;
	Style = const_cast<FQxMaskWidgetStyle2*>(InArgs._Style);

	SetCanTick(false);
}

int32 SQxMaskWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	throw std::logic_error("Not implemented");
}

FVector2D SQxMaskWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	throw std::logic_error("Not implemented");
}

bool SQxMaskWidget::IsInteractable() const
{
	return SLeafWidget::IsInteractable();
}

FReply SQxMaskWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return SLeafWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SQxMaskWidget::OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& GestureEvent)
{
	return SLeafWidget::OnTouchGesture(MyGeometry, GestureEvent);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
