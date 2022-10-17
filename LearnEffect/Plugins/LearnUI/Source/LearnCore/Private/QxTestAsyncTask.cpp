#include "QxTestAsyncTask.h"
#include "LearnCore.h"

void FQxTestAsyncTask::DoWork()
{
    double StartTime = FPlatformTime::Seconds();

    double Result = 0;
    for (int32 i = 0; i < TargetCounter; ++i)
    {
        Result += FMath::Sqrt(i) / TargetCounter;
    }

    UE_LOG(LearnCore, Warning, TEXT(__FUNCTION__"%u wait TargetCounterR(%f)"), __LINE__, Result);
    
    double EndTime = FPlatformTime::Seconds();
    UE_LOG(LearnCore, Warning, TEXT(__FUNCTION__"%u wait millsecond(%f) end."), __LINE__, (EndTime - StartTime)*1000);
}
