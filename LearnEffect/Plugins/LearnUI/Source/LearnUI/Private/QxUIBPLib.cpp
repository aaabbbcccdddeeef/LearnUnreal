// Fill out your copyright notice in the Description page of Project Settings.


#include "QxUIBPLib.h"


// 实现参考https://github.com/YawLighthouse/UMG-Slate-Compendium
TSharedPtr<FSlateUser> UQxUIBPLib::GetCurrentSlateUser(UObject* WorldContext)
{
	// The way you get the local player is dependent on your own needs
	// This is meant to be an example,
	// NOT INTENDED FOR FINAL/RELEASE CODE
	ULocalPlayer* localPlayer = nullptr;
	if(UWorld* const world = WorldContext->GetWorld())
	{
		if(APlayerController* const pc = world->GetFirstPlayerController())
		{
			localPlayer = pc->GetLocalPlayer();
		}
	}
	// Valid check the local player pointer
	if(!IsValid(localPlayer))
	{
		// If its not valid then exit the function
		return nullptr;
	}
	// Make sure Slate is initialized and working properly 
	if(FSlateApplication::IsInitialized())
	{
		// This function will return the Slate User that this local player is tied to
		return  FSlateApplication::Get().GetUser(localPlayer->GetControllerId());
	}
	return  nullptr;
}
