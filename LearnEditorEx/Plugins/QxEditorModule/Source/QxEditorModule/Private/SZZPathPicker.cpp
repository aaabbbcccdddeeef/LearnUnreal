// Fill out your copyright notice in the Description page of Project Settings.


#include "SZZPathPicker.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "ZZMeshTool"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SZZPathPicker::Construct(const FArguments& InArgs)
{
	WeakParentWindow = InArgs._ParentWindow;
	
	bPressedOk = false;
	AssetFolderPath = InArgs._AssetPath;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	FPathPickerConfig PathPickerConfig;
	PathPickerConfig.DefaultPath = AssetFolderPath;
	PathPickerConfig.OnPathSelected = FOnPathSelected::CreateRaw(this, &SZZPathPicker::OnSelectAssetPath);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig)
		]
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.Padding(0, 20, 0, 0)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(0, 2, 6, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(VAlign_Bottom)
				.ContentPadding(FMargin(8, 2, 8, 2))
				.OnClicked(this, &SZZPathPicker::OnClickOk)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
				.Text(LOCTEXT("OkButtonText", "OK"))
			]
			+ SHorizontalBox::Slot()
			.Padding(0, 2, 0, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(VAlign_Bottom)
				.ContentPadding(FMargin(8, 2, 8, 2))
				.OnClicked(this, &SZZPathPicker::OnClickCancel)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
				.Text(LOCTEXT("CancelButtonText", "Cancel"))
			]
		]
	];
}

FReply SZZPathPicker::OnClickOk()
{
	bPressedOk = true;

	if (WeakParentWindow.IsValid())
	{
		WeakParentWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SZZPathPicker::OnClickCancel()
{
	if (WeakParentWindow.IsValid())
	{
		WeakParentWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE