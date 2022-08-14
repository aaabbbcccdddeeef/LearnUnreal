// Fill out your copyright notice in the Description page of Project Settings.


#include "SEventTest.h"

#include <stdexcept>

#include "SlateOptMacros.h"
#include "Components/VerticalBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FQxPanToolModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SEventTest::Construct(const FArguments& InArgs)
{
	OnTestDelegate = InArgs._OnStartTest;

	ChildSlot
	[
		SNew(SVerticalBox)
		//------------//
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(60.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TextDefaultValue0", "SnapCamera:"))
				]
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(150.f)
				[
					SAssignNew(TestButtonPtr, SButton)
					.OnClicked(this, &SEventTest::OnTestButtonClicked)
					.Text(LOCTEXT("Login", "SnapCameraButton:"))
				]
			]
		]
		//------------//

		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(150.f)
				.HeightOverride(20.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TextDefaultValue1", "SecondTestLine"))
				]
			]
		]
	];
}

FReply SEventTest::OnTestButtonClicked()
{
	OnTestDelegate.ExecuteIfBound(TEXT("aa"), TEXT("bb"));
	
	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef  LOCTEXT_NAMESPACE