// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(QxRender, Log, All);

class FQxRenderSeriesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void PostResolveSceneColor_RenderThread(FRHICommandListImmediate& FrhiCommandListImmediate,
		FSceneRenderTargets& SceneRenderTargets);
	// Call this when you want to hook onto the renderer and start drawing. The shader will be executed once per frame.
	void BeginRendering();

	// When you are done, call this to stop drawing.
	void EndRendering();

	static inline FQxRenderSeriesModule& Get()
	{
		return FModuleManager::Get().LoadModuleChecked<FQxRenderSeriesModule>("QxRenderSeries");
	}

	static inline bool IsAvailable()
	{
		return  FModuleManager::Get().IsModuleLoaded("QxRenderSeries");
	}
private:
	FDelegateHandle OnPostResolvedSceneColorHandle;
};
