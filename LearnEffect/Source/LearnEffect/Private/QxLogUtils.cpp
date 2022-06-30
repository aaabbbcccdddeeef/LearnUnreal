// Fill out your copyright notice in the Description page of Project Settings.


#include "QxLogUtils.h"

#define LOCTEXT_NAMESPACE "KismetRenderingLibrary"

void UQxLogUtils::LogTest(UObject* WorldContextObject, const FName& InLogName)
{
	FMessageLog(InLogName).Warning(
		FText::Format(
			LOCTEXT("TestMessageLog", "Test Message Log in [{0}]: ."),
			FText::FromString(GetPathNameSafe(WorldContextObject))));
}

#undef LOCTEXT_NAMESPACE