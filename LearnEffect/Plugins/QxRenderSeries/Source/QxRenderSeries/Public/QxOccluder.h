// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxOccluder.generated.h"

struct FBoxOccluder
{
	FVector4 MinLocation;
	FVector4 MaxLocation;
};



BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FQxOccluderParameters,)
	// SHADER_PARAMETER_RDG_BUFFER_UAV()
	SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, MinLocationBuffer)
	// #TODO 先侧一个
	// SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, MaxLocationBuffer)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

// BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FShadowDepthPassUniformParameters,)
// 		SHADER_PARAMETER_UAV(RWStructuredBuffer<VplListEntry>, RWGvListBuffer)
// 	SHADER_PARAMETER_UAV(RWByteAddressBuffer, RWGvListHeadBuffer)
// 	
// 	SHADER_PARAMETER_UAV(RWByteAddressBuffer, RWVplListHeadBuffer)
// END_GLOBAL_SHADER_PARAMETER_STRUCT()

class UBoxComponent;
UCLASS()
class QXRENDERSERIES_API AQxOccluder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AQxOccluder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	void UploadBoxOccluderBuffers();

	void UploadBoxOccluderBuffers_RDG();
protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="QxRenderSeries")
	UBoxComponent* BoxComponent = nullptr;

private:
	TArray<FBoxOccluder> BoxOccluders;


};
