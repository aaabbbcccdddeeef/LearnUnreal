// Fill out your copyright notice in the Description page of Project Settings.


#include "QxClipperInfosMaterialExpression.h"


#if WITH_EDITOR
int32 UQxClipperInfosMaterialExpression::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
    // 这里无法确定MaterialTranslator
    return Super::Compile(Compiler, OutputIndex);
}

void UQxClipperInfosMaterialExpression::GetCaption(TArray<FString>& OutCaptions) const
{
    OutCaptions.Add(TEXT("QxClipperInfos"));
}
#endif

