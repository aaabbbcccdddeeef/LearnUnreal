#pragma once

class FQxReporterTask
{
    FString TaskName;
    AActor* TaskOwner;
public:
    FQxReporterTask(
        const FString& InName,
        AActor* InActor)
            : TaskName(InName)
            , TaskOwner(InActor)
    {
    }

    ~FQxReporterTask() {  }

    inline TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FQxReporterTask, STATGROUP_TaskGraphTasks);
    }

    static ENamedThreads::Type GetDesiredThread()
    {
        return ENamedThreads::AnyThread;
    }

    static  ESubsequentsMode::Type GetSubsequentsMode()
    {
        return ESubsequentsMode::TrackSubsequents;
    }

    void DoTask(ENamedThreads::Type CurrentThead, const FGraphEventRef& MyCompletionGraphEvent);
};
