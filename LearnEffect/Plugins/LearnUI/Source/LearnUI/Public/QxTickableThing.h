// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Stats/Stats.h"
#include "Tickable.h"

/**
 * 
 */
class LEARNUI_API FQxTickableThing : public FTickableGameObject 
{
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FQxTickableThing, STATGROUP_Tickables)
	};

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	};

	virtual bool IsTickableInEditor() const override
	{
		return false;
	};

	virtual bool IsTickableWhenPaused() const override
	{
		return true;
	};

private:

	uint32 LastFrameNumTicked = INDEX_NONE;
};
