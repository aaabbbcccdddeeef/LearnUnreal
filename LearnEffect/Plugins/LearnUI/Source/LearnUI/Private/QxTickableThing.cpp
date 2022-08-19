// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTickableThing.h"

void FQxTickableThing::Tick(float DeltaTime)
{
	if (LastFrameNumTicked == GFrameCounter)
	{

	}


	LastFrameNumTicked = GFrameCounter;
}


