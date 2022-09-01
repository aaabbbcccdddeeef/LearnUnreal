// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestCommands.h"
#define LOCTEXT_NAMESPACE "FQxTestTool"


void FQxTestCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "QxTestTool", "Bring up QxTestTool Window", EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Alt | EModifierKey::Shift, EKeys::T)); // 尝试定义一个快捷键
	UI_COMMAND(DoNothing, "QxDoNothing", "Do Nothing but just show", EUserInterfaceActionType::Button, FInputChord());
}
#undef LOCTEXT_NAMESPACE
