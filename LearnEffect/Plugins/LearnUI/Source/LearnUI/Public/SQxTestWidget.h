// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AHUD;
/**
 * 
 */
class LEARNUI_API SQxTestWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQxTestWidget)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	// Begin SWidget interface
	void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// end Swidget interface

private:
	TWeakObjectPtr<AHUD> OwnerHUD;
}
;
