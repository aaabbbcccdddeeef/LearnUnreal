// Fill out your copyright notice in the Description page of Project Settings.


#include "SWidgetDemo.h"

#include <stdexcept>

#include "LevelEditorActions.h"
#include "QxRenderUtils.h"
#include "QxRenderUtils.h"
#include "SlateOptMacros.h"
#include "Engine/Selection.h"
#include "Kismet/KismetSystemLibrary.h"
#define LOCTEXT_NAMESPACE "FQxPanToolModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SWidgetDemo::Construct(const FArguments& InArgs)
{
	ChildSlot
	.HAlign(HAlign_Left)
	[
		SNew(SEventTest).OnStartTest(this, &SWidgetDemo::OnMyTest)
	];
}



void SWidgetDemo::OnMyTest(FString usn, FString pwd)
{
	FLevelEditorActionCallbacks::SnapObjectToView_Clicked();

	for(FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		Actor->Modify();
		// Actor->Destroy();

		FHitResult OutHit;
		FQxRenderUtils::RayCastHit(Actor->GetActorLocation(), FVector(0, 0, -1), 10000.f, OutHit, Actor);
		
		FVector HitPointLoc = OutHit.Location + OutHit.ImpactNormal * 0.1f;
		Actor->SetActorLocation(HitPointLoc);
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


#undef  LOCTEXT_NAMESPACE