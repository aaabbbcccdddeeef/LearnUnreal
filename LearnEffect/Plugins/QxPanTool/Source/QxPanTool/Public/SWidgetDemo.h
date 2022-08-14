// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SEventTest.h"
#include "Engine/EngineTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class QXPANTOOL_API SWidgetDemo : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWidgetDemo)
	{
	}
	SLATE_EVENT(FTestDelegate, OnStartTest);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	bool RayCastHit(const FVector& RayOrigin, const FVector& RayDirection,
		float RayMarchingLength,  FHitResult& OutHitResult, AActor* InActor);
	void OnMyTest(FString usn, FString pwd);
};
