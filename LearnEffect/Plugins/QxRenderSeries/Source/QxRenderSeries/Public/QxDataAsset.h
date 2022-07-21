// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QxDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class QXRENDERSERIES_API UQxDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UQxDataAsset(const FObjectInitializer& InObjectInitializer);

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description", DisplayName = "Description", meta = (multiline = "true"))
		FText Description;*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QxRenderSeries")
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QxRenderSeries")
	float Test1;

	virtual void PostInitProperties() override;

	virtual void PostLoad() override;
private:
	FString PrintObjectInfo();
	
};
