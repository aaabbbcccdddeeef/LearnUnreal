// Copyright Epic Games, Inc. All Rights Reserved.

#include "QxEditorModule.h"

#include "ColorStructDetail.h"
#include "Framework/Commands/Commands.h"
#include "LevelEditor.h"
#include "QxAssetDetail.h"
#include "QxTestCommands.h"
#include "ToolMenus.h"
#include "PropertyEditorDelegates.h"
#include "QxDataAsset.h"

#define LOCTEXT_NAMESPACE "FQxEditorModuleModule"

DEFINE_LOG_CATEGORY(QxToolLog);

static const FName QxTestToolTabName("QxPanTool");

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
	//ExtendMenuItem2();

	// ExtendToolBar();
	//ExtendToolBar2();

	FGlobalTabmanager::Get()->RegisterTabSpawner(QxTestToolTabName,
		FOnSpawnTab::CreateRaw(this, &FQxEditorModuleModule::OnSpawnPluginTab));

	// // FQxAssetDetail tmp;
	//注册绑定
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout( UQxDataAsset::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FQxAssetDetail::MakeInstance));

	PropertyEditorModule.RegisterCustomPropertyTypeLayout("ColorStruct",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FColorStructDetail::MakeInstance));
	
	// 通知自定义模块修改完成
	PropertyEditorModule.NotifyCustomizationModuleChanged();
	
	

	// PropertyEditorModule.
	//通知自定义模块修改完成
}




void FQxEditorModuleModule::AddMenuExtension(FMenuBuilder& MenuBuilder)
{
	UE_LOG(QxToolLog, Warning, TEXT("Test Add MenuBarItem Extension"));
	// MenuBuilder.AddMenuEntry()
	MenuBuilder.AddMenuEntry(FQxTestCommands::Get().OpenPluginWindow);
}


void FQxEditorModuleModule::ExtendMenuItem()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{	// 创建菜单拓展项，就是菜单栏内的子项
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		// 增加菜单到 General UI扩展点之后
		// 注意： "VisitForum" ExtensionHook这个不是自定义的，而是根据已有的扩展点来看的，可以通过修改editor 设置看到
		MenuExtender->AddMenuExtension("VisitForums", EExtensionHook::After,
			PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FQxEditorModuleModule::AddMenuExtension));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
}

void FQxEditorModuleModule::ExtendMenuItem2()
{
	FToolMenuOwnerScoped ownerScoped(this);
	{
		UToolMenu* menu =UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			// 在Window Layout 部分添加
			FToolMenuSection& section = menu->FindOrAddSection("WindowLayout");
			section.AddMenuEntryWithCommandList(FQxTestCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}
}

void FQxEditorModuleModule::ExtendToolBar()
{
	FLevelEditorModule& editorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	// 创建工具栏扩展项
	{
		TSharedPtr<FExtender> ToolBarExtender = MakeShareable(new FExtender());

		ToolBarExtender->AddToolBarExtension("Settings", EExtensionHook::After,
			PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this, &FQxEditorModuleModule::AddToolBarExtension));

		editorModule.GetToolBarExtensibilityManager()->AddExtender(ToolBarExtender);
	}



}

void FQxEditorModuleModule::ExtendToolBar2()
{
	UToolMenu* toolBarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	{
		FToolMenuSection& section = toolBarMenu->FindOrAddSection("Settings");
		{
			FToolMenuEntry& entry = section.AddEntry(FToolMenuEntry::InitToolBarButton(
				FQxTestCommands::Get().OpenPluginWindow));
			entry.SetCommandList(PluginCommands);
		}
	}
}

void FQxEditorModuleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout(UQxDataAsset::StaticClass()->GetFName());
	PropertyEditorModule.UnregisterCustomPropertyTypeLayout("ColorStruct");
}

void FQxEditorModuleModule::PluginButtonClicked()
{
	UE_LOG(QxToolLog, Warning, TEXT("Plugin Button Clicked"));

	// 激活一个全局的tab窗口
	FGlobalTabmanager::Get()->TryInvokeTab(QxTestToolTabName);
}

void FQxEditorModuleModule::AddToolBarExtension(FToolBarBuilder& InBuilder)
{
	UE_LOG(QxToolLog, Warning, TEXT("Test Add Toolbar Extension"));

	//一个空的下拉菜单
	auto ContextMenu = []()
	{
		FMenuBuilder menuBuilder(true, nullptr);
		return menuBuilder.MakeWidget();
	};

	// 添加一个工具栏按钮
	InBuilder.AddToolBarButton(FQxTestCommands::Get().OpenPluginWindow);

	// 添加一个下拉菜单
	InBuilder.AddComboButton(FUIAction(),
		FOnGetContent::CreateLambda(ContextMenu),
		TAttribute<FText>(),
		TAttribute<FText>());

}

TSharedRef<class SDockTab> FQxEditorModuleModule::OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQxEditorModuleModule, QxEditorModule)