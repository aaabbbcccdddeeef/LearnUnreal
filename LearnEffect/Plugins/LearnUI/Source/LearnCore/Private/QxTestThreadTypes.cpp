#include "QxTestThreadTypes.h"


#include "LearnCore.h"
#include "QxMultithreadTest.h"
#include "QxTestRunnable.h"
#include "Kismet/KismetStringLibrary.h"

TAutoConsoleVariable<int32> CVarMultiTestIndex(
TEXT("r.QxMulti.TestIndex"),
4,
TEXT("CVarMultiTestIndex"),
ECVF_RenderThreadSafe 
);

constexpr int32 PrintStep = 1000;

FCriticalSection FQxTestRunnable::CriticalSection;
int32 FQxTestRunnable::ExpectNextThreadIndex = 0;

TAtomic<int32> FQxTestRunnable::bToUpdateThreadIndex = 0;

FQxTestRunnable::FQxTestRunnable(
    FString InThreadName,
    TPromise<float>&& InPromise,
    AQxTestRunnable* InTester,
    FQxTestShared* InTestShared)
    : MyThreadName(InThreadName), Tester(InTester), TestShared(InTestShared), TestPromise(MoveTemp(InPromise))
    , OtherTestRunnable(nullptr)
{
    FString tmp = MyThreadName.Right(1);
    ThreadIndex = UKismetStringLibrary::Conv_StringToInt(tmp);
}

bool FQxTestRunnable::Init()
{
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable {%s} Init"), *MyThreadName);
    // UpdateEvent->Reset();
    // UpdateEvent->Wait(MAX_uint32);
    if (CVarMultiTestIndex.GetValueOnAnyThread() == 2)
    {
        UpdateEvent->Wait();
    }

    int32 test = 100;
    return FRunnable::Init();
}

uint32 FQxTestRunnable::Run()
{
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable {%s} Run"), *MyThreadName);

    // InterleaveUpdate2();
    int32 TestIndex = CVarMultiTestIndex.GetValueOnAnyThread();

    switch (TestIndex)
    {
    case 1:
        InterleaveUpdate1();
        break;
    case 2:
        InterleaveUpdate2();
        break;
    case 3:
        InterleaveUpdate3();
        break;
    case 4:
        InterleaveUpdate4();
        break;
    case 5:
        InterleaveUpdate5();
        break;
    case 6:
        InterleaveUpdate6();
        break;
    }
    
    OtherTestRunnable = nullptr;
    TestPromise.SetValue(TestIndex + 10);
    return 0;
}

void FQxTestRunnable::Stop()
{
    
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable {%s} stop"), *MyThreadName);
    FRunnable::Stop();
}

void FQxTestRunnable::Exit()
{
   
    // OtherTestRunnable = nullptr;
    // Tester = nullptr;
    // TestShared = nullptr;
    UE_LOG(LearnCore, Warning, TEXT("QxTestRunnable{%s} Exit"), *MyThreadName);
    FRunnable::Exit();
}

FQxTestRunnable::~FQxTestRunnable()
{
    OtherTestRunnable = nullptr;
    Tester = nullptr;
    TestShared = nullptr;
}

void FQxTestRunnable::InterleaveUpdate1()
{
    while (IsValid(Tester))
    {
        FScopeLock Lock(&FQxTestRunnable::CriticalSection);
        bool bCurrentThreadIsExpectedThread = MyThreadName.Contains(FString::FromInt(ExpectNextThreadIndex));
        if (!bCurrentThreadIsExpectedThread)
        {
            continue;
        }
        FQxTestRunnable::ExpectNextThreadIndex = (ExpectNextThreadIndex + 1) % FQxTestRunnable::MaxThreadCount;
        
        if (Tester->TestCounter < Tester->CounterMax
            || Tester->bNeverStop)
        {
            Tester->TestCounter++;
            if (Tester->TestLogLevel == EQxTestLog::LogVerbose)
            {
                UE_LOG(LearnCore, Warning, TEXT("ThreadName: {%s} TestCounter = %d"), *MyThreadName, Tester->TestCounter);
            }
        }
        else
        {
            break;
        }
    }
}

void FQxTestRunnable::InterleaveUpdate2()
{
    UE_LOG(LearnCore, Warning, TEXT("ThreadName Test method 2 "));
    while (IsValid(Tester))
    {
        // FScopeLock Lock(&FQxTestRunnable::CriticalSection);
        FQxTestRunnable::CriticalSection.Lock();

        
        if (Tester->TestCounter < Tester->CounterMax
            || Tester->bNeverStop)
        {
            if (Tester->TestLogLevel == EQxTestLog::LogVerbose
                &&
                ((Tester->TestCounter) % PrintStep == ThreadIndex))
            {
                UE_LOG(LearnCore, Warning, TEXT("ThreadName: {%s} TestCounter = %d"), *MyThreadName, Tester->TestCounter);
            }
            Tester->TestCounter++;
            

            if (OtherTestRunnable)
            {
                OtherTestRunnable->UpdateEvent->Trigger();
            }
            const bool  bNeedWait = (Tester->TestCounter < Tester->CounterMax
                    || Tester->bNeverStop);
            CriticalSection.Unlock();

            // 
            if (bNeedWait)
            {
                UpdateEvent->Wait();
            }
        }
        else
        {
            CriticalSection.Unlock();
            break;
        }
    }
}

void FQxTestRunnable::InterleaveUpdate3()
{
    UE_LOG(LearnCore, Warning, TEXT("ThreadName Test method 3 "));
    while (true)
    {
        while (bToUpdateThreadIndex.Load() != ThreadIndex)
        {
        }

        if (TestShared->AtoTestCounter.Load() < TestShared->CounterMax
            || TestShared->bNeverStop)
        {
#pragma region PrintLog
            if (TestShared->TestLogLevel == EQxTestLog::LogVerbose)// &&
            {
                UE_LOG(LearnCore, Warning, TEXT("ThreadName: {%s} TestCounter = %d"),
                    *MyThreadName, int32(TestShared->AtoTestCounter));
            }
            else if (
                TestShared->TestLogLevel == EQxTestLog::LogSparse 
                && (TestShared->AtoTestCounter) % PrintStep == ThreadIndex
                )
            {
                UE_LOG(LearnCore, Warning, TEXT("ThreadName: {%s} TestCounter = %d"),
                    *MyThreadName, int32(TestShared->AtoTestCounter));
            }
#pragma endregion

            ++TestShared->AtoTestCounter;

            int32 nextUpdateIndex = (bToUpdateThreadIndex + 1) % 2;
            bToUpdateThreadIndex.Store(nextUpdateIndex);
        }
        else
        {
            break;
        }
    }
}

void FQxTestRunnable::InterleaveUpdate4()
{
    UE_LOG(LearnCore, Warning, TEXT("ThreadName Test method 4 "));
    while (true)
    {
        while (bToUpdateThreadIndex.Load(EMemoryOrder::Relaxed) != ThreadIndex)
        {
        }

        if (TestShared->AtoTestCounter.Load(EMemoryOrder::Relaxed) < TestShared->CounterMax
            || TestShared->bNeverStop)
        {
#pragma region PrintLog
            const int32 TmpCounter = TestShared->AtoTestCounter.Load(EMemoryOrder::Relaxed);
            if (TestShared->TestLogLevel == EQxTestLog::LogVerbose)// &&
                {
                UE_LOG(LearnCore, Warning, TEXT("ThreadName: {%s} TestCounter = %d"),
                    *MyThreadName, TmpCounter);
                }
            else if (
                TestShared->TestLogLevel == EQxTestLog::LogSparse 
                && (TmpCounter % PrintStep == ThreadIndex)
                )
            {
                UE_LOG(LearnCore, Warning, TEXT("ThreadName: {%s} TestCounter = %d"),
                    *MyThreadName, TmpCounter);
            }
#pragma endregion

            ++TestShared->AtoTestCounter;

            int32 nextUpdateIndex = (bToUpdateThreadIndex + 1) % 2;
            bToUpdateThreadIndex.Store(nextUpdateIndex, EMemoryOrder::Relaxed);
        }
        else
        {
            break;
        }
    }
}

void FQxTestRunnable::InterleaveUpdate5()
{
}

void FQxTestRunnable::InterleaveUpdate6()
{
}
