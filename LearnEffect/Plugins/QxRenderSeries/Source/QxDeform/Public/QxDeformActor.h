// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxDeformActor.generated.h"

class UQxDeformComponent;
UCLASS()
class QXDEFORM_API AQxDeformActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxDeformActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(EditAnywhere, Category="QxRender")
	UQxDeformComponent* DeformComponent;
	
	// 最终会从这个mesh 创建一个mesh section
	UPROPERTY(EditAnywhere, Category="QxRender")
	UStaticMesh* TestMesh;

	// 用来控制deform
	UPROPERTY(EditAnywhere, Category="QxRender")
	AActor* DeformController;
};
