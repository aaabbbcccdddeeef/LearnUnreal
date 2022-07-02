// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QxMultiOccluder.generated.h"

class UTextureRenderTarget2D;

// box形状的occluder需要用到的
USTRUCT(BlueprintType)
struct FBoxOccluderInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
	FVector Center;
	UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
	FVector BoxExtent;
	UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
	FVector ForwardVector;
	UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
	FVector UpVector;
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QXRENDERSERIES_API UQxMultiOccluder : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQxMultiOccluder();

	
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void  CollectOccluderInfos(TArray<FBoxOccluderInfo>&  OutOccluderInfos, int32& OutOccluderNum);

	UFUNCTION(BlueprintImplementableEvent, Category="MultiOccluder")
	TArray<FBoxOccluderInfo>  CollectOccluderInfos_BP();

	UFUNCTION(BlueprintCallable, Category="MultiOccluder")
	// 收集场景中的occluder 信息，并更新 texture
	void CollectOccluderAndUpdateTexture();

	UFUNCTION(BlueprintCallable, Category="MultiOccluder")
	void CreateTexture();
private:
	void UpdateOccluderTexture_RenderThread(UTexture2D* InOccluderTexture,
		int32 InOccluderNum,
		TArray<FBoxOccluderInfo>& InOccluderInfos);

protected:

	// occluders的参数 或存储在这张 纹理中，并提交到GPU, 当前只考虑box occluder， 要求occluder texture 只能是nx4 大小的
	UPROPERTY(Transient, EditAnywhere, BlueprintReadOnly, Category="MultiOccluder")
	UTexture2D* OccluderTexture;
};
