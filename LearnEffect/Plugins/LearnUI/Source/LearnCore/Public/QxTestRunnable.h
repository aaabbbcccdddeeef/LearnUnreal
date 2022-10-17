// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxTestEnums.h"
#include "QxTestThreadTypes.h"
#include "HAL/Runnable.h"
#include "QxTestRunnable.generated.h"


class FQxTestShared;
UCLASS(Blueprintable)
class LEARNCORE_API AQxTestRunnable : public AActor
{
	GENERATED_BODY()
public:
	AQxTestRunnable();
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="QxMultithread")
	void KillTestThreads(bool WaitFinish = false);

	UFUNCTION(BlueprintCallable, Category="QxMultithread")
	void SusppendTestThreads();

	UFUNCTION(BlueprintCallable, Category="QxMultithread")
	void ResumeTestThreads();

	int32 TestCounter = 0;

	UPROPERTY(EditAnywhere, Category="QxMultithread")
	int32 CounterMax = 100000;

	
	// bool bPrintRunLog = false;

	UPROPERTY(EditAnywhere, Category="QxMultithread")
	EQxTestLog TestLogLevel = EQxTestLog::NoLog;
	
	UPROPERTY(EditAnywhere, Category="QxMultithread")
	bool bNeverStop = false;

	UPROPERTY(EditAnywhere, Category="QxMultithread")
	bool bWaitThreadComplete = false;

	FQxTestShared* TestShared = nullptr;


	//#TODO 响应子线程完成任务
	// 1. 通过共享标志变量
	// 2. 通过event
	// 创建子线程后的两种处理方法，阻塞当前线程，不推荐，通过其他方式响应子线程完成
	
private:
	FRunnableThread*  RunnableThread1 = nullptr;
	FRunnableThread* RunnableThread2 = nullptr;

	TFuture<float> TestFuture1;
	TFuture<float> TestFuture2;
};


