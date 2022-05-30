// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "QxAssetFactory.generated.h"

/**
 * 
 */
UCLASS()
class QXRENDERSERIESEDITOR_API UQxAssetFactory : public UFactory
{
	GENERATED_BODY()
public:
	UQxAssetFactory();



	virtual UObject* FactoryCreateNew(
		UClass* InClass, 
		UObject* InParent, 
		FName InName, 
		EObjectFlags Flags, 
		UObject* Context, FFeedbackContext* Warn) override;


	virtual FText GetDisplayName() const override;



	virtual FString GetDefaultNewAssetName() const override;

};
