// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxClipperInfosMaterialExpression.generated.h"

/**
 * 
 */
UCLASS(collapsecategories, hidecategories=Object)
class QXSHADERREADER_API UQxClipperInfosMaterialExpression : public UMaterialExpression
{
    GENERATED_BODY()

#if WITH_EDITOR
    virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;

    virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
    
};
