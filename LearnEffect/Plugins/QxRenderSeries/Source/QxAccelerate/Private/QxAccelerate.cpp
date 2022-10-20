#include "QxAccelerate.h"

DEFINE_LOG_CATEGORY(QxAccelerate);

#define LOCTEXT_NAMESPACE "FQxAccelerate"

void FQxAccelerate::StartupModule()
{
	UE_LOG(QxAccelerate, Warning, TEXT("QxAccelerate module has been loaded"));
}

void FQxAccelerate::ShutdownModule()
{
	UE_LOG(QxAccelerate, Warning, TEXT("QxAccelerate module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxAccelerate, QxAccelerate)