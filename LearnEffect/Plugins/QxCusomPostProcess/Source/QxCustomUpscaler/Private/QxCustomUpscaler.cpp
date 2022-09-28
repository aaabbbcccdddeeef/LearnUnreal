#include "QxCustomUpscaler.h"

#include "SceneViewExtension.h"

DEFINE_LOG_CATEGORY(QxCustomUpscaler);


#define LOCTEXT_NAMESPACE "FQxCustomUpscaler"
void FQxCustomUpscaler::StartupModule()
{
	UE_LOG(QxCustomUpscaler, Warning, TEXT("QxCustomUpscaler module has been loaded"));
}

void FQxCustomUpscaler::ShutdownModule()
{
	UE_LOG(QxCustomUpscaler, Warning, TEXT("QxCustomUpscaler module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxCustomUpscaler, QxCustomUpscaler)