// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxSubsystem.generated.h"

/**
 * 测试实现一个subsystem,在UE4中实现单例的一种方式
 */
UCLASS()
class QXRENDERSERIES_API UQxSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return  true;
	};

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable)
	void AddScore(float delta);

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="QxSubsystem")
	float Score;
};


// 尝试定义一个类似C++风格的singleton,
// 但实践中不推荐这样做
UCLASS()
class UQxTestSingleton : public UObject
{
	GENERATED_BODY()
public:
	UQxTestSingleton(const FObjectInitializer& ObjectInitializer);

	virtual  void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category="QxRenderSeries")
	static UQxTestSingleton* GetInstance();

	UFUNCTION(BlueprintCallable, Category="QxRenderSeries")
	int32 Test();
private:
	static UQxTestSingleton* Instance;
};
