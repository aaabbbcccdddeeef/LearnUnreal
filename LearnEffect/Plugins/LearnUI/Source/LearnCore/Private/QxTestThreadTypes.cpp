#include "QxTestThreadTypes.h"
#include "LearnCore.h"
#include "QxMultithreadTest.h"
#include "QxTestRunnable.h"

FCriticalSection FQxTestRunnable::CriticalSection;

bool FQxTestRunnable::Init()
{
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable Init"));
    return FRunnable::Init();
}

uint32 FQxTestRunnable::Run()
{
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable Run"));

    while (IsValid(Tester))
    {
        FScopeLock Lock(&FQxTestRunnable::CriticalSection);
        if (Tester->TestCounter < Tester->CounterMax)
        {
            Tester->TestCounter++;
            UE_LOG(LearnCore, Warning, TEXT("TestCounter = %d"), Tester->TestCounter);
        }
        else
        {
            if (Tester->bNeverStop)
            {
                break;
            }
        }
    }
    
    return 0;
}

void FQxTestRunnable::Stop()
{
    FRunnable::Stop();
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable stop"));
}

void FQxTestRunnable::Exit()
{
    FRunnable::Exit();
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable Exit"));
}

FQxTestRunnable::~FQxTestRunnable()
{
}
