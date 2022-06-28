// Fill out your copyright notice in the Description page of Project Settings.


#include "QxConfigTest.h"

#include "Kismet/GameplayStatics.h"


// Sets default values
AQxConfigTest::AQxConfigTest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxConfigTest::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxConfigTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AQxConfigTest::TestConfigRead()
{
	if (!GConfig)
	{
		return;
	}

	FString ValueReceived;
	GConfig->GetString(
		TEXT("/Script/Engine.WorldInfo"), //Section
		TEXT("GlobalDefaultGameType"), //Key
		ValueReceived,
		GGameIni //引擎初始化好的全局配置文件路径
		);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	check(PlayerController);
	PlayerController->ClientMessage("GlobalDefaultGameType");
	PlayerController->ClientMessage(ValueReceived);

	int32 IntValueReceived;
	GConfig->GetInt(
		TEXT("Core.System"),
		TEXT("MaxObjectsNotConsideredByGC"),
		IntValueReceived,
		GEngineIni
		);

	PlayerController->ClientMessage("MaxObjectsNotConsideredByGC");
	PlayerController->ClientMessage(FString::FromInt(IntValueReceived));
	
}

void AQxConfigTest::TestConfigWrite()
{
	if (!GConfig)
	{
		return;
	}

	// 添加新的section
	FString QxTestSection = "QxConfig.Core";

	GConfig->SetString(
		*QxTestSection,
		TEXT("RootDir"),
		TEXT("E:/UE4/IsAwesome"),
		GGameIni
		);

	GConfig->SetVector(
		*QxTestSection,
		TEXT("PlayerStartLocation"),
		FVector(0, 0, 512),
		GGameIni
		);

	GConfig->SetRotator(
		*QxTestSection,
		TEXT("SunRotation"),
		FRotator(-90, 0, 0),
		GGameIni
		);

	GConfig->Flush(false, GGameIni);
}

void AQxConfigTest::TestLoadConfig()
{
	LoadConfig();
}

void AQxConfigTest::TestSaveConfig()
{
	SaveConfig();
}

