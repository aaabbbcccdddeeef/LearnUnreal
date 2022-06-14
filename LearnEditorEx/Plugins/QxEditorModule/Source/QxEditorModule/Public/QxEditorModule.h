// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
class FUICommandList;

class FQxEditorModuleModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void PluginButtonClicked();

	void ExtendToolbar();
	void AddMenuExtension(FMenuBuilder& MenuBuilder);
	void ExtendMenuItem();
	void ExtendMenuItem2();
	void AddToolBarExtension(FToolBarBuilder& InBuilder);
private:
	TSharedPtr<FUICommandList> PluginCommands;
};
