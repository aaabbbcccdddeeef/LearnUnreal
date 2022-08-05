#include "QxCusomPostProcess.h"

#include "Interfaces/IPluginManager.h"

// DEFINE_LOG_CATEGORY(QxCusomPostProcess); 

#define LOCTEXT_NAMESPACE "FQxCusomPostProcess"

void FQxCusomPostProcessModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("QxCusomPostProcess module has been loaded"));
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("QxRenderSeries"))->GetBaseDir(), TEXT("Shaders"));
	// PluginShaderDir = FPaths::Combine(PluginShaderDir, TEXT("CustomPostProcess"));
	AddShaderSourceDirectoryMapping(TEXT("/QxPPShaders"), PluginShaderDir);
	
}

void FQxCusomPostProcessModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("QxCusomPostProcess module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxCusomPostProcessModule, QxCusomPostProcess)