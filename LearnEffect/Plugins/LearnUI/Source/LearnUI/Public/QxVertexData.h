// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "QxVertexData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct LEARNUI_API FQxVertexData : public FTableRowBase
{
	GENERATED_BODY()
public:
	FQxVertexData()
	{
		
	}

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FQxVertexData)
	// int32 VertexID1 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FQxVertexData)
	float VertexIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FQxVertexData)
	float VertexX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FQxVertexData)
	float VertexY = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FQxVertexData)
	float VertexZ = 0;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FQxVertexData)
	// float PrimitiveID = -1;
};
