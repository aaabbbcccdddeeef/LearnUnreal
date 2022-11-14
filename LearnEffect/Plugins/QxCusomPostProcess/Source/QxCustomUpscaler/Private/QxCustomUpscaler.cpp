#include "QxCustomUpscaler.h"

#include "SceneViewExtension.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(QxCustomUpscaler);


#define LOCTEXT_NAMESPACE "FQxCustomUpscaler"
void FQxCustomUpscaler::StartupModule()
{
	UE_LOG(QxCustomUpscaler, Warning, TEXT("QxCustomUpscaler module has been loaded"));

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("QxCusomPostProcess"))->GetBaseDir(), TEXT("Shaders"));
	// PluginShaderDir = FPaths::Combine(PluginShaderDir, TEXT("CustomPostProcess"));
	AddShaderSourceDirectoryMapping(TEXT("/QxAAShaders"), PluginShaderDir);
}

void FQxCustomUpscaler::ShutdownModule()
{
	UE_LOG(QxCustomUpscaler, Warning, TEXT("QxCustomUpscaler module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxCustomUpscaler, QxCustomUpscaler)