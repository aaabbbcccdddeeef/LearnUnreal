// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPlayerConroller.h"

#include <stdexcept>

#include "QxCheatManager.h"

AQxPlayerConroller::AQxPlayerConroller(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CheatClass = UQxCheatManager::StaticClass();
}
