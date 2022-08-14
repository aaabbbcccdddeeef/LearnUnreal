// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRoad.h"

#include "Components/SplineComponent.h"


// Sets default values
AQxRoad::AQxRoad()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SetRootComponent(RootComp);

	RoadsSplineCom = CreateDefaultSubobject<USplineComponent>(TEXT("Road Spline Component"));
	RoadsSplineCom->SetSplinePoints(RoadPoints, ESplineCoordinateSpace::Local, true);

	RoadsSplineCom->bDrawDebug = true;

	RoadsSplineCom->SetupAttachment(GetRootComponent());

	
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxRoad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxRoad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

