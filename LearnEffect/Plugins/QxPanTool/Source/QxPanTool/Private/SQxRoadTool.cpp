// Fill out your copyright notice in the Description page of Project Settings.


#include "SQxRoadTool.h"

#include <stdexcept>

#include "QxRenderUtils.h"
#include "SlateOptMacros.h"
#include "../../../../QxRenderSeries/Source/QxRenderSeries/Public/QxRoad.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "QxRoadTool"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQxRoadTool::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
#pragma region FirstVSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(120.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TextDefaultValue0", "SplinePointNum:"))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(60.f)
				[
					SNew(SSpinBox<int32>)
					.MaxValue(100)
					.MinValue(0)
					.OnValueChanged(this, &SQxRoadTool::OnSliderChanged)
				]
			]
			
		]

#pragma endregion

		+ SVerticalBox::Slot()
#pragma region 2VSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(60.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TextDefaultValue1", "SplineSpecLength:1"))
				]
			]
		]
#pragma endregion

		+ SVerticalBox::Slot()
#pragma region 3Slot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(120.f)
				[
					SAssignNew(ButtonPtr, SButton)
					.OnClicked(this, &SQxRoadTool::OnButtonClicked)
					.Text(LOCTEXT("Test", "GenerateSpline"))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(20.f)
				.WidthOverride(120.f)
				[
					SAssignNew(ButtonPtr, SButton)
					.OnClicked(this, &SQxRoadTool::OnResetSplineButtonClicked)
					.Text(LOCTEXT("Test2", "ResetSpline"))
				]
			]
		]

#pragma endregion
	];
}

void SQxRoadTool::OnSliderChanged(int32 Value)
{
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		Actor->Modify();

		AQxRoad* road = Cast<AQxRoad>(Actor);
		if (road)
		{
			road->RoadPoints.Init(FVector::ZeroVector, Value);
		}
	}
}

void SQxRoadTool::OnSliderSecLengthChanged(float Value)
{
	RoadSecLength = Value;
}

FReply SQxRoadTool::OnButtonClicked()
{
	GenerateSpline();
	return FReply::Handled();
}

FReply SQxRoadTool::OnResetSplineButtonClicked()
{
	GenerateSpline();
	return FReply::Handled();
}

void SQxRoadTool::GenerateSpline()
{
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		Actor->Modify();

		AQxRoad* road = Cast<AQxRoad>(Actor);

		if (!road || road->RoadPoints.Num() < 1)
		{
			return;;
		}

		// find the first point of the spline
		FHitResult hitResult;
		FQxRenderUtils::RayCastHit(road->GetActorLocation(), FVector(0, 0, -1), 100.f, hitResult, road);

		road->RoadPoints[0] = hitResult.ImpactPoint;
		FVector LastPointLoc = hitResult.ImpactPoint;
		FVector RayStart = hitResult.ImpactPoint;
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef  LOCTEXT_NAMESPACE