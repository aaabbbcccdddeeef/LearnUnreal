#include "QxClippedStaticMesh.h"

#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(QxClippedStaticMesh);

#define LOCTEXT_NAMESPACE "FQxClippedStaticMesh"

void FQxClippedStaticMesh::StartupModule()
{
	UE_LOG(QxClippedStaticMesh, Warning, TEXT("QxClippedStaticMesh module has been loaded"));

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("QxRenderSeries"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/QxClipMeshShaders"), PluginShaderDir);
}

void FQxClippedStaticMesh::ShutdownModule()
{
	UE_LOG(QxClippedStaticMesh, Warning, TEXT("QxClippedStaticMesh module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxClippedStaticMesh, QxClippedStaticMesh)