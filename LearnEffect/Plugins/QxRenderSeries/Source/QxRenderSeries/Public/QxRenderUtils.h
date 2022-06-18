﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

/**
 * 
 */
class QXRENDERSERIES_API FQxRenderUtils 
{
public:
      static TArray<FVector> GetVertexPositonsWS(UStaticMeshComponent* InMeshComponent);
};
