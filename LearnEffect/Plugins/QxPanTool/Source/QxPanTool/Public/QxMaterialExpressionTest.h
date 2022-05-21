// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "QxMaterialExpressionTest.generated.h"

/**
 * 
 */
UCLASS()
class QXPANTOOL_API UQxMaterialExpressionTest : public UMaterialExpression
{
	GENERATED_BODY()
	
public:

private:

#if WITH_EDITOR
	int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;

	void GetCaption(TArray<FString>& OutCaptions) const override;

#endif

};
