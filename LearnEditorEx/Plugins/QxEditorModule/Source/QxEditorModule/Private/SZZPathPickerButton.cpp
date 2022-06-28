// Fill out your copyright notice in the Description page of Project Settings.


#include "SZZPathPickerButton.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "SlateOptMacros.h"
#include "SZZPathPicker.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FQxEditorModuleModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SZZPathPickerButton::Construct(const FArguments& InArgs)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	AssetFolderPath = ContentBrowserModule.Get().GetCurrentPath();
	ChildSlot
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("...")))
		.OnClicked(this, &SZZPathPickerButton::OnPathPickerSummoned)
	];
}

FReply SZZPathPickerButton::OnPathPickerSummoned()
{
	// Create the window to pick the class
	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(LOCTEXT("CreateBlueprintFromActors_PickPath", "Select Path"))
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(300.f, 400.f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SZZPathPicker> PathPickerDialog = SNew(SZZPathPicker)
		.ParentWindow(PickerWindow)
		.AssetPath(AssetFolderPath);

	PickerWindow->SetContent(PathPickerDialog);

	GEditor->EditorAddModalWindow(PickerWindow);

	if (PathPickerDialog->bPressedOk)
	{
		AssetFolderPath = PathPickerDialog->AssetFolderPath;
		UpdateFilenameStatus();
	}

	return FReply::Handled();
}

void SZZPathPickerButton::UpdateFilenameStatus()
{
	
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE