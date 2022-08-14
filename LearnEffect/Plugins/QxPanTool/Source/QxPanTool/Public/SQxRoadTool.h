// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class QXPANTOOL_API SQxRoadTool : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQxRoadTool)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void OnSliderChanged(int32	Value);
	void OnSliderSecLengthChanged(float Value);
	
	TSharedPtr<SButton> ButtonPtr;
	
	FReply OnButtonClicked();
	FReply OnResetSplineButtonClicked();
	
	float RoadSecLength;
	
private:
	void GenerateSpline();

	
};
