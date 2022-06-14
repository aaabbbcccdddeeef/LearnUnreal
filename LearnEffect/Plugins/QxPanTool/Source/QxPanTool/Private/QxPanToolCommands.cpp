// Copyright Epic Games, Inc. All Rights Reserved.

#include "QxPanToolCommands.h"

#define LOCTEXT_NAMESPACE "FQxPanToolModule"

void FQxPanToolCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "QxPanTool", "Bring up QxPanTool window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
