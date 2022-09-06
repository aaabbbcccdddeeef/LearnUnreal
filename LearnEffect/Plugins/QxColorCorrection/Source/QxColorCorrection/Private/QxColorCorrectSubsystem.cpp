// Fill out your copyright notice in the Description page of Project Settings.


#include "QxColorCorrectSubsystem.h"
#include "QxColorCorrection.h"
#include "QxColorCorrectRegion.h"
#include "QxColorCorrectSceneViewExtension.h"
#include "SceneViewExtension.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

static bool IsRegionValid(AQxColorCorrectRegion* InRegion, UWorld* InWorld)
{
	// There some cases in which actor can belong to a different world or the world without subsystem.
	// Example: when editing a blueprint deriving from AVPCRegion.
	// We also check if the actor is being dragged from the content browser.
#if WITH_EDITOR
	return InRegion && !InRegion->bIsEditorPreviewActor && (InRegion->GetWorld() == InWorld);
#else
	return InRegion && InRegion->GetWorld() == InWorld;	
#endif
}

void UQxColorCorrectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Super::Initialize(Collection);

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		GEngine->OnLevelActorAdded().AddUObject(this, &UQxColorCorrectSubsystem::OnActorSpawned);
		GEngine->OnLevelActorDeleted().AddUObject(this, &UQxColorCorrectSubsystem::OnActorDeleted);
		GEngine->OnLevelActorListChanged().AddUObject(this, &UQxColorCorrectSubsystem::OnLevelActorListChanged);
		GEditor->RegisterForUndo(this);
	}
#endif

	GetWorld()->OnLevelsChanged().AddUObject(this, &UQxColorCorrectSubsystem::OnLevelsChanged);

	QxColorCorrectSceneViewExtension = FSceneViewExtensions::NewExtension<FQxColorCorrectSceneViewExtension>(this);
}

void UQxColorCorrectSubsystem::Deinitialize()
{
	// Super::Deinitialize();
#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		GEngine->OnLevelActorAdded().RemoveAll(this);
		GEngine->OnLevelActorDeleted().RemoveAll(this);
		GEngine->OnLevelActorListChanged().RemoveAll(this);
		GEditor->UnregisterForUndo(this);
	}
#endif

	GetWorld()->OnLevelsChanged().RemoveAll(this);
	for (AQxColorCorrectRegion* Region : Regions)
	{
		Region->ClearUp();
	}
	Regions.Reset();

	// Prevent this SVE from being gathered, in case it is kept alive by a strong reference somewhere else.
	{
		QxColorCorrectSceneViewExtension->IsActiveThisFrameFunctions.Empty();

		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;
		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
		{
			return TOptional<bool>(false);
		};

		QxColorCorrectSceneViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}

	QxColorCorrectSceneViewExtension.Reset();
	QxColorCorrectSceneViewExtension = nullptr;
}



void UQxColorCorrectSubsystem::OnActorSpawned(AActor* InActor)
{
	AQxColorCorrectRegion* QxCCRegion = Cast<AQxColorCorrectRegion>(InActor);
	if (IsRegionValid(QxCCRegion, GetWorld()))
	{
		FScopeLock RegionScopeLock(&RegionAccessCriticalSection);

		if (!Regions.Contains(QxCCRegion))
		{
			Regions.Add(QxCCRegion);
			SortRegionsByPriority();
		}
	}
}

void UQxColorCorrectSubsystem::OnActorDeleted(AActor* InActor)
{
	AQxColorCorrectRegion* QxCCRegion = Cast<AQxColorCorrectRegion>(InActor);
	if (QxCCRegion
#if WITH_EDITOR
		&& !QxCCRegion->bIsEditorPreviewActor)
#else
	)
#endif
	{
		FScopeLock RegionScopeLock(&RegionAccessCriticalSection);

		Regions.Remove(QxCCRegion);

		
	}
}

void UQxColorCorrectSubsystem::RefreshRegions()
{
}

void UQxColorCorrectSubsystem::SortRegionsByPriority()
{
	FScopeLock RegionScopeLock(&RegionAccessCriticalSection);

	Regions.Sort([](const AQxColorCorrectRegion& A, const AQxColorCorrectRegion& B)
		{
			return A.Priority < B.Priority;
		}
		);
}
