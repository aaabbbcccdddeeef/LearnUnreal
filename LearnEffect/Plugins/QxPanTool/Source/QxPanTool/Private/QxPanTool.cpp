// Copyright Epic Games, Inc. All Rights Reserved.

#include "QxPanTool.h"
#include "QxPanToolStyle.h"
#include "QxPanToolCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "MyFile_BPLib.h"

static const FName QxPanToolTabName("QxPanTool");

#define LOCTEXT_NAMESPACE "FQxPanToolModule"

void FQxPanToolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FQxPanToolStyle::Initialize();
	FQxPanToolStyle::ReloadTextures();

	FQxPanToolCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FQxPanToolCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FQxPanToolModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FQxPanToolModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(QxPanToolTabName, FOnSpawnTab::CreateRaw(this, &FQxPanToolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FQxPanToolTabTitle", "QxPanTool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FQxPanToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FQxPanToolStyle::Shutdown();

	FQxPanToolCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(QxPanToolTabName);
}

TSharedRef<SDockTab> FQxPanToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FQxPanToolModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("QxPanTool.cpp"))
		);

	FOnClicked testOnClicked;
	testOnClicked.BindStatic(&UMyFile_BPLib::CreateTestTexture);

	FOnClicked testOnClicked2;
	testOnClicked2.BindStatic(&UMyFile_BPLib::CreateTestTexture2);

	FOnClicked testOnClicked3;
	testOnClicked3.BindStatic(&UMyFile_BPLib::CreateTestTexture3);
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			//// Put your tab content here!
			//SNew(SBox)
			//.HAlign(HAlign_Center)
			//.VAlign(VAlign_Center)
			//[
			//	SNew(STextBlock)
			//	.Text(WidgetText)
			//]
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SButton)
				.OnClicked(testOnClicked) //&UMyFile_BPLib::CreateTestTexture)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CreateTestTexture", "CreateTestTexture"))
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SButton)
				.OnClicked(testOnClicked2) //&UMyFile_BPLib::CreateTestTexture)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CreateTestTexture2", "CreateTestTexture2"))
				]
			]
			+ SVerticalBox::Slot()
			[
					SNew(SButton)
					.OnClicked(testOnClicked3) //&UMyFile_BPLib::CreateTestTexture)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CreateTestTexture3", "CreateTestTexture3"))
				]
			]
		];
}

void FQxPanToolModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(QxPanToolTabName);
}

void FQxPanToolModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FQxPanToolCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FQxPanToolCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQxPanToolModule, QxPanTool)