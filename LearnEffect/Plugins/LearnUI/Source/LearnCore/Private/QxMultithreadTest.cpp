// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMultithreadTest.h"
#include <mutex>

#if PLATFORM_WINDOWS
// #include "Windows.h"
#endif


DECLARE_STATS_GROUP(TEXT("QxMultThread"), STATGROUP_QxMultiThread, STATCAT_Advanced); //定义命令名称，其中STATGROUP_QxTestGroup是stat命令输入的文字， TEXT内是显示的，STATCAT_Advanced是固定写法
// DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("qx stat count"), STAT_COUNT, STATGROUP_QxTestGroup);
DECLARE_CYCLE_STAT(TEXT("QxTestDevide"), STAT_QxTestDevide, STATGROUP_QxMultiThread); //统计时间, STAT_MyTIME 是宏名和SCOPE_CYCLE_COUNTER关联
DECLARE_CYCLE_STAT(TEXT("QxCriticalSectionOverhead"), STAT_QxCriticalSectionOverhead, STATGROUP_QxMultiThread);
DECLARE_CYCLE_STAT(TEXT("QxMutexOverhead"), STAT_QxMutexOverhead, STATGROUP_QxMultiThread);
DECLARE_CYCLE_STAT(TEXT("QxStdMutexhead"), STAT_QxStdMutexhead, STATGROUP_QxMultiThread);

static  constexpr int32 TestLoop = 1000000; 

// Sets default values
AQxMultithreadTest::AQxMultithreadTest()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}


void AQxMultithreadTest::PrintTestData()
{
    for (float Element : TestData1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Test"));
    }
}

void AQxMultithreadTest::TestDived(TArray<float>& InArray)
{
    SCOPE_CYCLE_COUNTER(STAT_QxTestDevide);
    ParallelFor(InArray.Num(), [&](int32 Index)
    {
        InArray[Index] /= 10;
        if (Index % 100 == 0)
        {
            // FScopeLock LockGuard1(&TestMutex);
            TestMutex.Lock();
            Test5Sum += InArray[Index];
            TestMutex.Unlock();
        }
    }
    , EParallelForFlags::None);
    UE_LOG(LogTemp, Warning, TEXT("Test"));
}

void AQxMultithreadTest::Test()
{
    GenerateTestData1();
    TestDived(TestData1);
}

void AQxMultithreadTest::TestCriticalSectionOverhead()
{
    float Sum = 0;
    {
        SCOPE_CYCLE_COUNTER(STAT_QxCriticalSectionOverhead);
        for (int32 i = 0; i < TestLoop; i++)
        {
            TestMutex.Lock();
            // for (int32 j = 0; j < 100; ++j)
            // {
            //     Sum += 1;
            // }
            TestMutex.Unlock();
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("critical section overhead = %f"), Sum);
    
}

void AQxMultithreadTest::TestMutexOverhead()
{
#if PLATFORM_WINDOWS
    // float Sum = 0;
    // HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
    // {
    //     SCOPE_CYCLE_COUNTER(STAT_QxMutexOverhead);
    //     for (int i = 0; i < 1000000; i++)
    //     {
    //         WaitForSingleObject(mutex, INFINITE);
    //         for (int32 j = 0; j < 100; ++j)
    //         {
    //             Sum += 1;
    //         }
    //         ReleaseMutex(mutex);
    //     }
    // }
    // CloseHandle(mutex);
    // UE_LOG(LogTemp, Warning, TEXT("mutex overhead = %f"), Sum);
#endif

}

void AQxMultithreadTest::TestStdMutexOverhead()
{
    float Sum = 0;
    // HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
    std::mutex lock1;
    {
        SCOPE_CYCLE_COUNTER(STAT_QxStdMutexhead);
        for (int i = 0; i < TestLoop; i++)
        {
            lock1.lock();
            // for (int32 j = 0; j < 100; ++j)
            // {
            //     Sum += 1;
            // }
            lock1.unlock();
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("mutex overhead = %f"), Sum);
}

// Called when the game starts or when spawned
void AQxMultithreadTest::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AQxMultithreadTest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Test();

    if (bEnableOverheadTest)
    {
        TestCriticalSectionOverhead();
        TestMutexOverhead();
        TestStdMutexOverhead();
    }
}

void AQxMultithreadTest::GenerateTestData1()
{
    {
        // IsInRenderingThread();
        // TArray<float> TestData;
        TestData1.Empty();
        
        const int32 TestSize = 1000000;
        TestData1.Reserve(TestSize);
        // TestData1.AddUninitialized(TestSize);
        for (int i = 0; i < TestSize; ++i)
        {
            // TestData1[i] = i;
            TestData1.Add(i);
        }
    }

    Test5Sum = 0;
}

