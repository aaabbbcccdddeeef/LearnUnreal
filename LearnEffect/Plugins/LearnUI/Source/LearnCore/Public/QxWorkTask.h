#pragma once

class FQxWorkTask
{
    FString TaskName;
    TArray<TGraphTask<FQxWorkTask>*> ChildTasks;
    AActor* TaskOwner;
public:
    FQxWorkTask(
        const FString& InName,
        TArray<TGraphTask<FQxWorkTask>*> Children,
        AActor* InActor)
            : TaskName(InName)
            , ChildTasks(MoveTemp(Children))
            , TaskOwner(InActor)
    {
    }

    ~FQxWorkTask()
    {
    }

    inline TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FQxWorkTask, STATGROUP_TaskGraphTasks);
    }

    static ENamedThreads::Type GetDesiredThread()
    {
        return ENamedThreads::AnyThread;
    }

    static ESubsequentsMode::Type GetSubsequentsMode()
    {
        return ESubsequentsMode::TrackSubsequents;
    }

    void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent);
};
