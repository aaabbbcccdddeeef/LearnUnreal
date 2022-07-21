// Fill out your copyright notice in the Description page of Project Settings.


#include "QxSubsystem.h"

UQxTestSingleton* UQxTestSingleton::Instance = nullptr;

void UQxSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UQxSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UQxSubsystem::AddScore(float delta)
{
	Score += delta;
}

UQxTestSingleton::UQxTestSingleton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 不设置singleton instance 在CDO中或者其他 unreal style constructors
	if (!HasAnyFlags(RF_ClassDefaultObject
		| RF_NeedLoad 
		| RF_NeedPostLoad
		| RF_NeedPostLoadSubobjects
		))
	{
		ensureMsgf( Instance == nullptr, TEXT( "Uh-oh two singletons!" ) );
		Instance = this;
	}
}

void UQxTestSingleton::BeginDestroy()
{
	if (Instance != nullptr)
	{
		if (ensureMsgf(Instance == this, TEXT("We have a single ton that is not this")))
		{
			Instance = nullptr;
		}
	}
	Super::BeginDestroy();
}

UQxTestSingleton* UQxTestSingleton::GetInstance()
{
	
	if (Instance == nullptr)
	{
		NewObject<UQxTestSingleton>();
		// ensureMsgf(Instance, TEXT("Construct begin access"));
	}
	return Instance;
}

int32 UQxTestSingleton::Test()
{
	static int32 test = 0;
	return  ++test;
}
