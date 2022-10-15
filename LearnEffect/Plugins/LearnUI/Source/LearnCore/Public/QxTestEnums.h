// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxTestEnums.generated.h"

UENUM(BlueprintType)
enum class EQxTestLog : uint8
{
    NoLog,
    LogVerbose,
    LogSparse
};
