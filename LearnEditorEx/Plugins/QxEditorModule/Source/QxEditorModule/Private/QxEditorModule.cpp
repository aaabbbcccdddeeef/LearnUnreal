// Copyright Epic Games, Inc. All Rights Reserved.

#include "QxEditorModule.h"
#include "Framework/Commands/Commands.h"
#include "LevelEditor.h"
#include "QxTestCommands.h"

#define LOCTEXT_NAMESPACE "FQxEditorModuleModule"


void FQxEditorModuleModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// 加载LevelEditor模块
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	FQxTestCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(FQxTestCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FQxEditorModuleModule::PluginButtonClicked));
	// PluginCommands->MapAction()
	ExtendMenuItem();
}


void FQxEditorModuleModule::ExtendToolbar()
{
	TSharedPtr<FExtender> MenuBarExtender = MakeShareable(new FExtender());
		
	// FToolBarExtensionDelegate& tmpDelegate =
	// 	;
	MenuBarExtender->AddToolBarExtension("SettingsXX", EExtensionHook::After,
	                                     PluginCommands, 
	                                     FToolBarExtensionDelegate::CreateRaw(this, &FQxEditorModuleModule::AddToolBarExtension));
}

void FQxEditorModuleModule::AddMenuExtension(FMenuBuilder& MenuBuilder)
{
	UE_LOG(LogTemp, Warning, TEXT("Test Add MenuBarItem Extension"));
	// MenuBuilder.AddMenuEntry()
	MenuBuilder.AddMenuEntry(FQxTestCommands::Get().OpenPluginWindow);
}


void FQxEditorModuleModule::ExtendMenuItem()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{	// 创建菜单拓展项，就是菜单栏内的子项
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		// 增加菜单到 General UI扩展点之后
		MenuExtender->AddMenuExtension("VisitForums", EExtensionHook::After,
			PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FQxEditorModuleModule::AddMenuExtension));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
}

void FQxEditorModuleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FQxEditorModuleModule::PluginButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Plugin Button Clicked"));

}

void FQxEditorModuleModule::AddToolBarExtension(FToolBarBuilder& InBuilder)
{
	UE_LOG(LogTemp, Warning, TEXT("Test Add Toolbar Extension"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQxEditorModuleModule, QxEditorModule)