// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "QxPanToolStyle.h"

class FQxPanToolCommands : public TCommands<FQxPanToolCommands>
{
public:

	FQxPanToolCommands()
		: TCommands<FQxPanToolCommands>(TEXT("QxPanTool"), NSLOCTEXT("Contexts", "QxPanTool", "QxPanTool Plugin"), NAME_None, FQxPanToolStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};