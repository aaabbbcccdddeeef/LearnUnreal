#include "ZZMultiClipper.h"

DEFINE_LOG_CATEGORY(ZZMultiClipper);

#define LOCTEXT_NAMESPACE "FZZMultiClipper"

void FZZMultiClipper::StartupModule()
{
	UE_LOG(ZZMultiClipper, Warning, TEXT("ZZMultiClipper module has been loaded"));
}

void FZZMultiClipper::ShutdownModule()
{
	UE_LOG(ZZMultiClipper, Warning, TEXT("ZZMultiClipper module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZZMultiClipper, ZZMultiClipper)