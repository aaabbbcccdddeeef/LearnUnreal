// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxTestAsyncTask.h"
#include "GameFramework/Actor.h"
#include "QxTestAsyncTaskActor.generated.h"

UCLASS()
class LEARNCORE_API AQxTestAsyncTaskActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AQxTestAsyncTaskActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // 测试用auto delete 的方法创建task
    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestAsyncTaskClass1();

    // 测试用FAsyncTask的方法创建task，需要手动管理销毁
    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestAsyncTaskClass2();

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestAsyncTaskFunc_AnyThread();

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
   void TestAsyncTaskFunc_GameThread();

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestAsyncFunc_NoReturn();

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestAsyncFunc_WithFuture();

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void TestAsyncFunc_ParallelFor();

    void OnAsyncFuncFinish();
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QxMultiThread")
    int TargetCounter = 100;

private:
    double StartTime = 0;

    // FAsyncTask 这个对象是线程安全的吗，我能够在主线程用这个控制task 吗?
    TUniquePtr<FAsyncTask<FQxTestAsyncTask>> TestAsyncTask;
};

