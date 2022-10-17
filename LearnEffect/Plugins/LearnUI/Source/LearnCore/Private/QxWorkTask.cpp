#include "QxWorkTask.h"

#include <stdexcept>

#include "LearnCore.h"
#include "QxReporterTask.h"

void FQxWorkTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{

    UE_LOG(LearnCore, Warning, TEXT(__FUNCTION__ " %s Begin"), *TaskName);
    for (TGraphTask<FQxWorkTask>* Task : ChildTasks)
    {
        if (Task)
        {
            Task->Unlock();
            MyCompletionGraphEvent->DontCompleteUntil(Task->GetCompletionEvent());
        }
    }

    // do something
    MyCompletionGraphEvent->DontCompleteUntil(
        TGraphTask<FQxReporterTask>::CreateTask().ConstructAndDispatchWhenReady(
            TaskName, TaskOwner)
        );
    UE_LOG(LearnCore, Warning, TEXT(__FUNCTION__ " %s:End"), *TaskName);
}
