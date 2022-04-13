// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Input/Reply.h"
#include "MyFile_BPLib.generated.h"

/**
 * 
 */
UCLASS()
class QXPANTOOL_API UMyFile_BPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FReply CreateTestTexture();

	static FReply CreateTestTexture2();

	static FReply CreateTestTexture3();
};
