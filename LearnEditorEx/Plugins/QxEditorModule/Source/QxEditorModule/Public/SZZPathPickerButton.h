// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SZZPathPicker;
/**
 * 
 */
class QXEDITORMODULE_API SZZPathPickerButton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZZPathPickerButton)
		{
		}

	SLATE_END_ARGS()

	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);


	
private:
	FReply OnPathPickerSummoned();

	void UpdateFilenameStatus();
	
public:
	FString AssetFolderPath;
private:

	TSharedPtr<SZZPathPicker> ZZPathPicker;
};
