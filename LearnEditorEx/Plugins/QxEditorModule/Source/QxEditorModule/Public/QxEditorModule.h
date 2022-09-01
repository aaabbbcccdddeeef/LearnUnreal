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

private:
	void PluginButtonClicked();
	void PluginButtonClicked2();

	void AddMenuExtension(FMenuBuilder& MenuBuilder);
	void ExtendMenuItem();
	void ExtendMenuItem2();
	void ExtendToolBar();
	void ExtendToolBar2();

	// 定义一个只有表现没有响应的按钮
	void ExtendToolWithNoAction();
	void AddToolBarExtension(FToolBarBuilder& InBuilder);
	

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	
	static void OnPathSelected(const FString& InPath);

	static FQxEditorModuleModule& Get();

	
private:
	TSharedPtr<FUICommandList> PluginCommands;

	TSharedPtr<FUICommandList> DoNothingCommands;

	FString PorxyMeshPath;
};
