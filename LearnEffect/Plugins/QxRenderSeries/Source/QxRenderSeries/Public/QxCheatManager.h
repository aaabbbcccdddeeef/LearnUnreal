// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxPlayerConroller.h"
#include "GameFramework/CheatManager.h"
#include "UObject/Object.h"
#include "QxCheatManager.generated.h"

/**
 * 
 */
UCLASS( Within = QxPlayerConroller)
class QXRENDERSERIES_API UQxCheatManager : public UCheatManager
{
	GENERATED_BODY()
public:
	UFUNCTION(Exec)
	void BuyCakes(int32 NumCakes, int32 Diameter = 1);

	UFUNCTION(Exec)
	void SetCakeName(FString NewName);


	// general purpose functions
	UFUNCTION(Exec)
	void ForceCrash();

	UFUNCTION(Exec)
	void ForceGarbageCollection();
};
