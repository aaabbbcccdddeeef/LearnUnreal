// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAssetFactory.h"
#include "QxDataAsset.h"

#define LOCTEXT_NAMESPACE "QxAssetFactory" 


UQxAssetFactory::UQxAssetFactory()
{
	bCreateNew = true;
	//bEditAfterNew = true;
	SupportedClass = UQxDataAsset::StaticClass();
}

UObject* UQxAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	//return Super::FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn);
	UQxDataAsset* dataAsset = NewObject<UQxDataAsset>(InParent, InClass, InName, Flags);
	return dataAsset;
}

FText UQxAssetFactory::GetDisplayName() const
{
	//return Super::GetDisplayName();
	return LOCTEXT("QxDataAsset", "QxDataAsset");
}

FString UQxAssetFactory::GetDefaultNewAssetName() const
{
	//return Super::GetDefaultNewAssetName();
	return FString(TEXT("NewQxDataAsset"));
}

