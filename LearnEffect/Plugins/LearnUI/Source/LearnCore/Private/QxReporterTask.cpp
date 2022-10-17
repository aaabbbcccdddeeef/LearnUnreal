#include "QxReporterTask.h"

#include <stdexcept>

#include "QxGraphTaskTestActor.h"

void FQxReporterTask::DoTask(ENamedThreads::Type CurrentThead, const FGraphEventRef& MyCompletionGraphEvent)
{
    AQxGraphTaskTestActor* TestActor = Cast<AQxGraphTaskTestActor>(TaskOwner);
    if (IsValid(TestActor))
    {
        TestActor->OnTaskComplete(TaskName);
    }
}
