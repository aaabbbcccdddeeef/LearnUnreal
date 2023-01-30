#include "LearnReflection.h"

DEFINE_LOG_CATEGORY(LearnReflection);

#define LOCTEXT_NAMESPACE "FLearnReflection"

void FLearnReflection::StartupModule()
{
	UE_LOG(LearnReflection, Warning, TEXT("LearnReflection module has been loaded"));
}

void FLearnReflection::ShutdownModule()
{
	UE_LOG(LearnReflection, Warning, TEXT("LearnReflection module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLearnReflection, LearnReflection)