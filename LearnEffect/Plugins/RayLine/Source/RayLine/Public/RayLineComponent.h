// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "RayLineComponent.generated.h"


USTRUCT(BlueprintType)
struct FRayLineHitPointDesc
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RayLine")
	FVector HitPosition;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RayLine")
	FVector HitNextDirection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RayLine")
	int32 HitPointIndex;
};

/**
 * 
 */
UCLASS(ClassGroup = "Rendering", meta = (BlueprintSpawnableComponent))
class RAYLINE_API URayLineComponent : public UMeshComponent
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RayLineComponent")
	float DebugSec ;

private:
#pragma region UE_Interfaces
	//~ Begin UPrimitiveComponent Interface;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface;

	//~ Begin UMeshComponent Interface;
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface;

	//~ Begin USceneComponent Interface;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface;

	//~ Begin UActorComponent Interface;
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	//~ End UActorComponent Interface;


#pragma endregion

public:
	UPROPERTY(VisibleAnywhere)
	TArray<FRayLineHitPointDesc> RayLineHitPoints;

	friend class FRayLineMeshSceneProxy;
};
