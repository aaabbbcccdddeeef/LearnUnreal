// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxGraphTaskTestActor.generated.h"


USTRUCT(BlueprintType)
struct FTaskItem
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    FString TaskName;

    FGraphEventRef GraphEvent;
    TGraphTask<class FQxWorkTask>* GraphTask;

    FTaskItem()
        : TaskName(TEXT("NoName"))
        , GraphEvent(nullptr)
        , GraphTask(nullptr)
    {
    }

    FTaskItem(const FString& InName, FGraphEventRef InEventRef = FGraphEventRef())
        : TaskName(InName), GraphEvent(InEventRef), GraphTask(nullptr)
    {
    }

    FTaskItem(const FString& InName, TGraphTask<FQxWorkTask>* Task = nullptr)
        : TaskName(InName), GraphEvent(nullptr), GraphTask(Task)
    {
    }

    ~FTaskItem()
    {
        GraphEvent = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class LEARNCORE_API AQxGraphTaskTestActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AQxGraphTaskTestActor();

    void OnTaskComplete(const FString& TaskName);
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    FTaskItem CreateTask(const FString& TaskName,
        const TArray<FTaskItem>& Prerequires,
        const TArray<FTaskItem>& ChildTasks,
        bool DispatchWhenReady = true);

    // 没有前置和child的task创建
    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    FTaskItem CreateTaskPure(const FString& TaskName,
        bool DispathWhenReady = true)
    {
        const TArray<FTaskItem> Empty;
        return  CreateTask(TaskName, Empty, Empty, DispathWhenReady);
    }

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    FTaskItem CreateTaskWithPrerequireOnly(
        const FString& TaskName,
        const TArray<FTaskItem>& Prerequires,
        bool bDispathWhenReady= true)
    {
        return CreateTask( TaskName, Prerequires, TArray<FTaskItem>(), bDispathWhenReady);
    }

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    FTaskItem CreateTaskWithChildTaskOnly(
        const FString& TaskName,
        const TArray<FTaskItem>& ChildTasks,
        bool bDispatchWhenReady = true)
    {
        return CreateTask(TaskName, TArray<FTaskItem>(), ChildTasks, bDispatchWhenReady);
    }

    UFUNCTION(BlueprintCallable, Category="QxMultiThread")
    void FireTask(const FTaskItem& TaskItem);


    
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};
