// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxColorCorrectRegion.generated.h"


/**
 * @brief  一个colorCorrectionRegion的instance。用来聚合所有校色区域
 * QxColorCorrectSubsystem定义这个类的actor示例的添加、删除等等
 * 
 */
UCLASS(Blueprintable)
class QXCOLORCORRECTION_API AQxColorCorrectRegion : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxColorCorrectRegion();

	void ClearUp();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	int32 Priority = 0;
};
