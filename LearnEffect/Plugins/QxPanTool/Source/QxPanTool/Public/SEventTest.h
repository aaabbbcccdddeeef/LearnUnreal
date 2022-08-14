// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SButton;
class SEditableTextBox;

DECLARE_DELEGATE_TwoParams(FTestDelegate, FString, FString);

/**
 * 
 */
class QXPANTOOL_API SEventTest : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEventTest)
	{
	}
	SLATE_EVENT(FTestDelegate, OnStartTest);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FReply OnTestButtonClicked();
	
private:

	TSharedPtr<SEditableTextBox> TestTextOnePtr;
	TSharedPtr<SEditableTextBox> TestTextTwoPtr;
	TSharedPtr<SButton> TestButtonPtr;

	FTestDelegate OnTestDelegate;

	
};
