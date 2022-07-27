// Fill out your copyright notice in the Description page of Project Settings.


#include "QxDeform/Public/QxDeformActor.h"

#include "QxDeform/Public/QxDeformComponent.h"


// Sets default values
AQxDeformActor::AQxDeformActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DeformComponent = CreateDefaultSubobject<UQxDeformComponent>(TEXT("QxDeformComponent"));
	DeformController = CreateDefaultSubobject<AActor>("DeformController");
}

// Called when the game starts or when spawned
void AQxDeformActor::BeginPlay()
{
	Super::BeginPlay();

	const FTransform& DeformTrans = DeformController->GetTransform();
	DeformComponent->CreateMeshSection(0, TestMesh, DeformTrans);
}

// Called every frame
void AQxDeformActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FTransform& DeformTrans = DeformController->GetTransform();
	DeformComponent->UpdateMeshSectionTransform(0, DeformTrans);
	DeformComponent->FinishTransformsUpdate();
	DeformComponent->MarkRenderStateDirty();
}

