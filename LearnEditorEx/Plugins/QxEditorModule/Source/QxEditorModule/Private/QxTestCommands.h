// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class FQxTestCommands : public TCommands<FQxTestCommands>
{
public:
	FQxTestCommands()
		: TCommands<FQxTestCommands>(TEXT("QxTestCommands"),
			NSLOCTEXT("Contexts", "QxTestTool", "QxTest Tool"),
			NAME_None, FEditorStyle::GetStyleSetName() )
	{
		
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;

	TSharedPtr<FUICommandInfo> DoNothing;
};
