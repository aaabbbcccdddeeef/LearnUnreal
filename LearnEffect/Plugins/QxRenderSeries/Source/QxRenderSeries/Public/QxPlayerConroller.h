// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxPlayerConroller.generated.h"

/**
 * 
 */
UCLASS()
class QXRENDERSERIES_API AQxPlayerConroller : public APlayerController
{
	GENERATED_BODY()
	friend class UQxCheatManager;
public:
	AQxPlayerConroller(const FObjectInitializer& ObjectInitializer);


protected:
	// key 是直径，value是数量
	UPROPERTY(BlueprintReadOnly, Category="QxRender")
	TMap<int32, int32> Cakes;

	UPROPERTY(BlueprintReadOnly, Category="QxRender")
	FName TestName;
};
