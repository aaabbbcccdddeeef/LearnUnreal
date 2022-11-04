// Copyright Epic Games, Inc. All Rights Reserved.

#include "QyRenderSeries.h"

#include "QxTestRenderer.h"

#define LOCTEXT_NAMESPACE "FQyRenderSeriesModule"

void FQyRenderSeriesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	ENQUEUE_RENDER_COMMAND(CreateQxTestRenderer)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			QxTestRenderer = MakeShared<FQxTestRenderer>();
		}
		);
}

void FQyRenderSeriesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQyRenderSeriesModule, QyRenderSeries)