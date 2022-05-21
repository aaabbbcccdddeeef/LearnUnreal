// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMaterialExpressionTest.h"
#include <MaterialCompiler.h>

#if WITH_EDITOR
int32 UQxMaterialExpressionTest::Compile(class FMaterialCompiler* Compiler, int32 OutputIndex)
{
	Super::Compile(Compiler, OutputIndex);
	//throw std::logic_error("The method or operation is not implemented.");
	return Compiler->Constant(0.536f);
}

void UQxMaterialExpressionTest::GetCaption(TArray<FString>& OutCaptions) const
{
	//Super::GetCaption(OutCaptions);
	//throw std::logic_error("The method or operation is not implemented.");
	OutCaptions.Add(TEXT("QxTest"));
}
#endif

