// Copyright Epic Games, Inc. All Rights Reserved.

#include "QxRenderSeries.h"
#include <Interfaces/IPluginManager.h>

#include "QxRenderBPLib.h"
#include "Renderer/Private/PostProcess/SceneRenderTargets.h"

DEFINE_LOG_CATEGORY(QxRender);

#define LOCTEXT_NAMESPACE "FQxRenderSeriesModule"

void FQxRenderSeriesModule::StartupModule()
{
	OnPostResolvedSceneColorHandle.Reset();
	
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("QxRenderSeries"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/QxShaders"), PluginShaderDir);

}

void FQxRenderSeriesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FQxRenderSeriesModule::PostResolveSceneColor_RenderThread(
	FRHICommandListImmediate& FrhiCommandListImmediate,
	FSceneRenderTargets& SceneRenderTargets)
{

	UQxRenderBPLib::PostResolveSceneColor_RenderThread(FrhiCommandListImmediate, SceneRenderTargets);
}

void FQxRenderSeriesModule::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this,
			&FQxRenderSeriesModule::PostResolveSceneColor_RenderThread);
	}
}

void FQxRenderSeriesModule::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}
	OnPostResolvedSceneColorHandle.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQxRenderSeriesModule, QxRenderSeries)