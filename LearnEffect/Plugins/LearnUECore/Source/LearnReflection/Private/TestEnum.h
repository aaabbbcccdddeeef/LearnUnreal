// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TestEnum.generated.h"

UENUM()
enum class TestEnum
{
    MY_Dance 	UMETA(DisplayName = "Dance"),
    MY_Rain 	UMETA(DisplayName = "Rain"),
    MY_Song		UMETA(DisplayName = "Song")
};
