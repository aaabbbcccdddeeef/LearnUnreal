#include "QxCustomMeshes.h"

#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(QxCustomMeshes);

#define LOCTEXT_NAMESPACE "FQxCustomMeshes"

void FQxCustomMeshes::StartupModule()
{
	UE_LOG(QxCustomMeshes, Warning, TEXT("QxCustomMeshes module has been loaded"));

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("QxRenderSeries"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/QxMeshShaders"), PluginShaderDir);
}

void FQxCustomMeshes::ShutdownModule()
{
	UE_LOG(QxCustomMeshes, Warning, TEXT("QxCustomMeshes module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxCustomMeshes, QxCustomMeshes)