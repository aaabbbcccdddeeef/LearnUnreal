// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPPActor.h"

#include "QxRenderSeries.h"


// Sets default values
AQxPPActor::AQxPPActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxPPActor::BeginPlay()
{
	Super::BeginPlay();

	FQxRenderSeriesModule::Get().BeginRendering();
}

void AQxPPActor::BeginDestroy()
{
	FQxRenderSeriesModule::Get().EndRendering();
	Super::BeginDestroy();
}

// Called every frame
void AQxPPActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

