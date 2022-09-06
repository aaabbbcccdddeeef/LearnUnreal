// Fill out your copyright notice in the Description page of Project Settings.


#include "QxColorCorrectRegion.h"

#include "QxColorCorrectRegionDatabase.h"
#include "QxColorCorrectSubsystem.h"


// Sets default values
AQxColorCorrectRegion::AQxColorCorrectRegion()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AQxColorCorrectRegion::ClearUp()
{
	QxColorCorrectSubsystem = nullptr;
}

// Called when the game starts or when spawned
void AQxColorCorrectRegion::BeginPlay()
{
	Super::BeginPlay();
	if (const UWorld* World = GetWorld())
	{
		QxColorCorrectSubsystem = World->GetSubsystem<UQxColorCorrectSubsystem>();
	}
	if (QxColorCorrectSubsystem)
	{
		QxColorCorrectSubsystem->OnActorSpawned(this);
	}
}

void AQxColorCorrectRegion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (QxColorCorrectSubsystem)
	{
		QxColorCorrectSubsystem->OnActorDeleted(this);
		QxColorCorrectSubsystem = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AQxColorCorrectRegion::BeginDestroy()
{
	if (QxColorCorrectSubsystem)
	{
		QxColorCorrectSubsystem->OnActorDeleted(this);
		QxColorCorrectSubsystem = nullptr;
	}
	Super::BeginDestroy();
}


void AQxColorCorrectRegion::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::Tick(DeltaTime);

	FTransform CurrentFrameTransform = GetTransform();
	if (!PreviousFrameTransform.Equals(CurrentFrameTransform))
	{
		PreviousFrameTransform = CurrentFrameTransform;
		GetActorBounds(true, BoxOrigin, BoxExtent);
	}

	if (const UPrimitiveComponent* FirstPrimitiveComponent = FindComponentByClass<UPrimitiveComponent>())
	{
		FQxColorCorrectRegionDatabase::UpdateCCRDatabaseFirstComponentId(this,
			FirstPrimitiveComponent->ComponentId);
	}
}

bool AQxColorCorrectRegion::ShouldTickIfViewportsOnly() const
{
	return true;
}


#ifdef WITH_EDITOR
void AQxColorCorrectRegion::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AQxColorCorrectRegion, Priority)
		|| (PropertyChangedEvent.Property == nullptr))
	{
		if (!QxColorCorrectSubsystem)
		{
			if (const UWorld* World = GetWorld())
			{
				QxColorCorrectSubsystem =
					World->GetSubsystem<UQxColorCorrectSubsystem>();
			}
		}

		if (QxColorCorrectSubsystem)
		{
			QxColorCorrectSubsystem->SortRegionsByPriority();
		}
	}

	GetActorBounds(true, BoxOrigin, BoxExtent);
}
#endif


