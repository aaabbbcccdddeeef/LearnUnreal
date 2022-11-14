// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZZCustomCull.h"

#include "ZZRenderer.h"

#define LOCTEXT_NAMESPACE "FZZCustomCullModule"

void FZZCustomCullModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	ENQUEUE_RENDER_COMMAND(CreateZZRenderer)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			ZZRenderer = MakeShared<FZZRenderer>();
		}
		);
}

void FZZCustomCullModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZZCustomCullModule, ZZCustomCull)