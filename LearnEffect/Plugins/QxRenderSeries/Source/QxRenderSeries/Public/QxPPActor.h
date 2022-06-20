// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxPPActor.generated.h"

UCLASS()
class QXRENDERSERIES_API AQxPPActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxPPActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
