// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxRuntimMeshOperate.generated.h"

UCLASS()
class QXRENDERSERIES_API AQxRuntimMeshOperate : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQxRuntimMeshOperate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="QxMesh")
	void GenerateMeshCom_Test1();

private:
	
	/**
	 * @brief 
	 * @param OutResources 应该是没有初始化的LODResources
	 * @param OutMeshBounds 这个是包括所有顶点的bounds
	 */
	void FillLODResourcesWithTestData1(FStaticMeshLODResources& OutResources, FBoxSphereBounds& OutMeshBounds);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
