// Fill out your copyright notice in the Description page of Project Settings.


#include "ZZPathPickerButton.h"

#include "SZZPathPickerButton.h"

FString UZZPathPickerButton::GetPickedPath() const
{
	if (PathPickerButton.IsValid())
	{
		return  PathPickerButton->AssetFolderPath;
	}
	return FString();
}

TSharedRef<SWidget> UZZPathPickerButton::RebuildWidget()
{
	PathPickerButton = SNew(SZZPathPickerButton);
	return PathPickerButton.ToSharedRef();
}

void UZZPathPickerButton::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	PathPickerButton.Reset();
}
