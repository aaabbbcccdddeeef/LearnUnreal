#include "LearnCore.h"

DEFINE_LOG_CATEGORY(LearnCore);

#define LOCTEXT_NAMESPACE "FLearnCore"

void FLearnCore::StartupModule()
{
	UE_LOG(LearnCore, Warning, TEXT("LearnCore module has been loaded"));
}

void FLearnCore::ShutdownModule()
{
	UE_LOG(LearnCore, Warning, TEXT("LearnCore module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLearnCore, LearnCore)