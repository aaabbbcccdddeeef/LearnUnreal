// Fill out your copyright notice in the Description page of Project Settings.


#include "QxGraphTaskTestActor.h"

#include <stdexcept>

#include "QxWorkTask.h"
#include "LearnCore.h"


// Sets default values
AQxGraphTaskTestActor::AQxGraphTaskTestActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxGraphTaskTestActor::BeginPlay()
{
    Super::BeginPlay();
    
}

FTaskItem AQxGraphTaskTestActor::CreateTask(const FString& TaskName, const TArray<FTaskItem>& Prerequires,
    const TArray<FTaskItem>& ChildTasks, bool DispatchWhenReady)
{
    FGraphEventArray PrerequireEvents;
    TArray<TGraphTask<FQxWorkTask>*> ChildEvents;
    UE_LOG(LearnCore, Warning, TEXT("Task [%s] is Created."), *TaskName);

    if (Prerequires.Num() > 0)
    {
        PrerequireEvents.Reserve(Prerequires.Num());
        for (const FTaskItem& Item : Prerequires)
        {
            if (Item.GraphTask)
            {
                PrerequireEvents.Add(Item.GraphTask->GetCompletionEvent());
                UE_LOG(LearnCore, Warning, TEXT("Task [%s] wait Task [%s]"), *TaskName, *Item.TaskName);
            } else if (Item.GraphEvent)
            {
                PrerequireEvents.Add(Item.GraphEvent);
                UE_LOG(LearnCore, Warning, TEXT("Task [%s] wait Task [%s]"), *TaskName, *Item.TaskName);
            }
        }
    }

    if (ChildTasks.Num() > 0)
    {
        ChildEvents.Reserve(ChildTasks.Num());
        for (const FTaskItem& Item : ChildTasks)
        {
            if (Item.GraphTask)
            {
                ChildEvents.Add(Item.GraphTask);
                UE_LOG(LearnCore, Warning, TEXT("Task [%s] will excute after Task[%s]"), *Item.TaskName, *TaskName);
            }
        }
    }

    if (DispatchWhenReady)
    {
        return FTaskItem(
            TaskName,
            TGraphTask<FQxWorkTask>::CreateTask(&PrerequireEvents).ConstructAndDispatchWhenReady(
                TaskName, ChildEvents, this)
            );
    }

    return  FTaskItem(
            TaskName,
            TGraphTask<FQxWorkTask>::CreateTask(&PrerequireEvents).ConstructAndHold(
                TaskName, ChildEvents, this)
        );
}

void AQxGraphTaskTestActor::FireTask(const FTaskItem& TaskItem)
{
    if (TaskItem.GraphTask)
    {
        UE_LOG(LearnCore, Warning, TEXT("Task [%s] Fire."), *TaskItem.TaskName);
        TaskItem.GraphTask->Unlock();
    }
}

void AQxGraphTaskTestActor::OnTaskComplete(const FString& TaskName)
{
    UE_LOG(LearnCore, Warning, TEXT("Task[%s] is Reporeted"), *TaskName);
}

// Called every frame
void AQxGraphTaskTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

