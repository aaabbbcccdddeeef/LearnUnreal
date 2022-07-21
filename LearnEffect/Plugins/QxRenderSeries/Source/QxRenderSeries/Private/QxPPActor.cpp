// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPPActor.h"

#include "QxRenderSeries.h"


// Sets default values
AQxPPActor::AQxPPActor(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UE_LOG(LogTemp, Warning, TEXT("AQxPPActor::Constructor"));
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

void AQxPPActor::PostInitProperties()
{
	Super::PostInitProperties();
	UE_LOG(LogTemp, Warning, TEXT("AQxPPActor::PostInitProperties"));
}

void AQxPPActor::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogTemp, Warning, TEXT("AQxPPActor::PostLoad"));
}

// Called every frame
void AQxPPActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

