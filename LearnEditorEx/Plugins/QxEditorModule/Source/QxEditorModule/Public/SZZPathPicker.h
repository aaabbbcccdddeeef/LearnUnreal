// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class QXEDITORMODULE_API SZZPathPicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZZPathPicker)
		{
		}
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_ARGUMENT(FString, AssetPath)
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Callback when the selected asset path has changed. */
	void OnSelectAssetPath(const FString& Path) { AssetFolderPath = Path; }
	
	/** Callback when the "ok" button is clicked. */
	FReply OnClickOk();

	/** Destroys the window when the operation is cancelled. */
	FReply OnClickCancel();

	/** A pointer to the window that is asking the user to select a parent class */
	TWeakPtr<SWindow> WeakParentWindow;
	
	FString AssetFolderPath;

	bool bPressedOk;
};







