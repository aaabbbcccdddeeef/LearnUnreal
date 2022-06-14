// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestCommands.h"
#define LOCTEXT_NAMESPACE "FQxTestTool"


void FQxTestCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "QxTestTool", "Bring up QxTestTool Window", EUserInterfaceActionType::Button, FInputChord());
}
#undef LOCTEXT_NAMESPACE
