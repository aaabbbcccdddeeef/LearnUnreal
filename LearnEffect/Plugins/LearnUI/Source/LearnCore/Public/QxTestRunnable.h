// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "QxTestRunnable.generated.h"




UCLASS()
class LEARNCORE_API AQxTestRunnable : public AActor
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="QxMultithread")
	void KillTestThreads();

	UFUNCTION(BlueprintCallable, Category="QxMultithread")
	void SusppendTestThreads();

	UFUNCTION(BlueprintCallable, Category="QxMultithread")
	void ResumeTestThreads();

	int32 TestCounter = 0;

	UPROPERTY(EditAnywhere, Category="QxMultithread")
	int32 CounterMax = 100000;

	UPROPERTY(EditAnywhere, Category="QxMultithread")
	bool bNeverStop = false;

	UPROPERTY(EditAnywhere, Category="QxMultithread")
	bool bWaitThreadComplete = false;

	
private:
	FRunnableThread*  RunnableThread1 = nullptr;
	FRunnableThread* RunnableThread2 = nullptr;
};


