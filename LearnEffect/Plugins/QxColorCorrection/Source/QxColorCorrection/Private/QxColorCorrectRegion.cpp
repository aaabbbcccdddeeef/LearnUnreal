// Fill out your copyright notice in the Description page of Project Settings.


#include "QxColorCorrectRegion.h"


// Sets default values
AQxColorCorrectRegion::AQxColorCorrectRegion()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AQxColorCorrectRegion::ClearUp()
{
}

// Called when the game starts or when spawned
void AQxColorCorrectRegion::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxColorCorrectRegion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

