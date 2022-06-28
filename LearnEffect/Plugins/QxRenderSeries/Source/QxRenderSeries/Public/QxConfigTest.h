// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxConfigTest.generated.h"

UCLASS(Config=Game)
class QXRENDERSERIES_API AQxConfigTest : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxConfigTest();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="QxRenderSeires")
	void TestConfigRead();

	UFUNCTION(BlueprintCallable, Category="QxRenderSeires")
	void TestConfigWrite();

	UFUNCTION(BlueprintCallable, Category="QxRenderSeires")
	void TestLoadConfig();

	UFUNCTION(BlueprintCallable, Category="QxRenderSeires")
	void TestSaveConfig();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Config, BlueprintReadWrite, Category="QxRenderSeires")
	FString TestStr;

	UPROPERTY(Config, BlueprintReadWrite, Category="QxRenderSeires")
	float TestFloat;
};
