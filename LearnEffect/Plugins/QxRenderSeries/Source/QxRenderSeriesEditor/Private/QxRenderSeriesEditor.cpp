// Copyright Epic Games, Inc. All Rights Reserved.

#include "QxRenderSeriesEditor.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName QxEditorTabName("QxEditor");

#define LOCTEXT_NAMESPACE "FQxEditorModule"

void FQxRenderSeriesEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

}

void FQxRenderSeriesEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQxRenderSeriesEditorModule, QxEditor)