// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#if WITH_EDITOR
#include "EditorUndoClient.h"
#endif

#include "QxColorCorrectSubsystem.generated.h"

/**
 * Conditional inheritance to allow UColorCorrectRegionsSubsystem to inherit/avoid Editor's Undo/Redo in Editor/Game modes.
 */ 
#if WITH_EDITOR
class FQxColorCorrectSceneViewExtension;
class AQxColorCorrectRegion;

class FQxColorCorrectRegionsEditorUndoClient : public  FEditorUndoClient
{
};
#else
class FQxColorCorrectRegionsEditorUndoClient 
{
};
#endif

/**
 * 这个world subsystem关联level中的AQxColorCorrectRegion的实例。
 * 包括level加载、undo/Redo、添加到关卡、移除等事件
 *
 * 通过引擎的一些回调追踪QxColorCorrectRegion实例， 包括 OnLevelActorAdded, OnLevelActorDeleted,OnLevelActorListChanged
 * 
 */
UCLASS()
class QXCOLORCORRECTION_API UQxColorCorrectSubsystem : public UWorldSubsystem, public FQxColorCorrectRegionsEditorUndoClient
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#if WITH_EDITOR
	virtual void PostUndo(bool bSuccess) override {RefreshRegions();};
	virtual void PostRedo(bool bSuccess) override {RefreshRegions();};
#endif

	// a call back for cc region creation
	void OnActorSpawned(AActor* InActor);

	// a call back for cc region deletion
	void OnActorDeleted(AActor* InActor);

	// called when level is added or removed 
	void OnLevelsChanged() {RefreshRegions();}

	void SortRegionsByPriority();
#if WITH_EDITOR
	void OnLevelActorListChanged() {RefreshRegions();}
#endif
private:
	// populate region actors
	void RefreshRegions();


public:

	/** Stores pointers to all ColorCorrectRegion Actors. */ 
	TArray<AQxColorCorrectRegion*> Regions;

private:

	// region class.用来获得关卡中所有的region actors
	TSubclassOf<AQxColorCorrectRegion> RegionClass;

	TSharedPtr<FQxColorCorrectSceneViewExtension, ESPMode::ThreadSafe> QxColorCorrectSceneViewExtension;

	FCriticalSection RegionAccessCriticalSection;

public:
	friend class FQxColorCorrectSceneViewExtension;
};
