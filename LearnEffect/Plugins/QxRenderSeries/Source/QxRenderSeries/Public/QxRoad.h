// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxRoad.generated.h"

class USplineComponent;
UCLASS()
class QXRENDERSERIES_API AQxRoad : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxRoad();


	USplineComponent* GetRoadSpline() const {return RoadsSplineCom;}
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

#pragma region Components
	UPROPERTY(VisibleAnywhere, Category="Road")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, Category="Road")
	USplineComponent* RoadsSplineCom;
#pragma endregion
	
	TArray<FVector> RoadPoints;
	
};
