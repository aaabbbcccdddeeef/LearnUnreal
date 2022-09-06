#include "QxColorCorrectEditor.h"

DEFINE_LOG_CATEGORY(QxColorCorrectEditor);

#define LOCTEXT_NAMESPACE "FQxColorCorrectEditor"

void FQxColorCorrectEditor::StartupModule()
{
	UE_LOG(QxColorCorrectEditor, Warning, TEXT("QxColorCorrectEditor module has been loaded"));
}

void FQxColorCorrectEditor::ShutdownModule()
{
	UE_LOG(QxColorCorrectEditor, Warning, TEXT("QxColorCorrectEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxColorCorrectEditor, QxColorCorrectEditor)