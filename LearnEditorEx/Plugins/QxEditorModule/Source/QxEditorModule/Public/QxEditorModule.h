// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
class FUICommandList;

DECLARE_LOG_CATEGORY_EXTERN(QxToolLog, Log, All);

class FQxEditorModuleModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void PluginButtonClicked();

	void AddMenuExtension(FMenuBuilder& MenuBuilder);
	void ExtendMenuItem();
	void ExtendMenuItem2();
	void ExtendToolBar();
	void ExtendToolBar2();
	void AddToolBarExtension(FToolBarBuilder& InBuilder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	
	static void OnPathSelected(const FString& InPath);

	static FQxEditorModuleModule& Get();
private:
	TSharedPtr<FUICommandList> PluginCommands;

	FString PorxyMeshPath;
};
