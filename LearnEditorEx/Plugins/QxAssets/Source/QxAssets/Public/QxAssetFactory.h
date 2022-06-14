// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "QxAssetFactory.generated.h"

/**
 * 
 */
UCLASS()
class QXASSETS_API UQxAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
	

public:
	UQxAssetFactory(FObjectInitializer& ObjectInitializer);

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent,
		FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

};
