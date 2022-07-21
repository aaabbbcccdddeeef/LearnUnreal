// Fill out your copyright notice in the Description page of Project Settings.


#include "QxCheatManager.h"

#include <stdexcept>

#include "QxPlayerConroller.h"

void UQxCheatManager::BuyCakes(int32 NumCakes, int32 Diameter)
{
	// APlayerController* const PC = ;
	AQxPlayerConroller* QxPC = Cast<AQxPlayerConroller>(GetOuterAPlayerController());
	if (QxPC)
	{
		if (QxPC->Cakes.Contains(Diameter))
		{
			QxPC->Cakes[Diameter] += NumCakes;
		}
		else
		{
			// QxPC->Cakes[Diameter] = NumCakes;
			QxPC->Cakes.Emplace(Diameter, NumCakes);
		}
		
	}
	UE_LOG(LogTemp, Warning, TEXT("Buy cakes invoked"));
}

void UQxCheatManager::SetCakeName(FString NewName)
{
	// APlayerController* const PC = ;
	AQxPlayerConroller* QxPC = Cast<AQxPlayerConroller>(GetOuterAPlayerController());
	if (QxPC)
	{
		QxPC->TestName = FName(NewName);
	}
	UE_LOG(LogTemp, Warning, TEXT("SetCakeName invoked"));
}

void UQxCheatManager::ForceCrash()
{
	check(false);
}

void UQxCheatManager::ForceGarbageCollection()
{
	GEngine->ForceGarbageCollection(true);
}
