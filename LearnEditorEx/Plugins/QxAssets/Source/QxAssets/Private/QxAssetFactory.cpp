// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAssetFactory.h"
#include "QxDataAsset.h"



UQxAssetFactory::UQxAssetFactory(FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;

	bEditAfterNew = true;

	SupportedClass = UQxDataAsset::StaticClass();
}

UObject* UQxAssetFactory::FactoryCreateNew(UClass* InClass,
	UObject* InParent, FName InName,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{

	check(InClass->IsChildOf(UQxDataAsset::StaticClass()));

	return NewObject<UQxDataAsset>(InParent, InClass, InName, Flags | RF_Transactional);
}
