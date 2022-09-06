// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * @brief 这个插件和模块的目的是参照UE4的ColorCorrectRegion 插件,
 * 尝试在插件中自定义后期
 */
class FQxColorCorrectionModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
