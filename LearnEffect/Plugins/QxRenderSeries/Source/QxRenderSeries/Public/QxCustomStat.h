// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxCustomStat.generated.h"

UCLASS()
class QXRENDERSERIES_API AQxCustomStat : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxCustomStat();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
