#include "QxUpScalerRigister.h"

DEFINE_LOG_CATEGORY(QxUpScalerRigister);

#define LOCTEXT_NAMESPACE "FQxUpScalerRigister"

void FQxUpScalerRigister::StartupModule()
{
	UE_LOG(QxUpScalerRigister, Warning, TEXT("QxUpScalerRigister module has been loaded"));

	QxUpscalerExtention = FSceneViewExtensions::NewExtension<FQxUpscalerExtention>();
}

void FQxUpScalerRigister::ShutdownModule()
{
	UE_LOG(QxUpScalerRigister, Warning, TEXT("QxUpScalerRigister module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxUpScalerRigister, QxUpScalerRigister)