// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMultithreadTest.h"

DECLARE_STATS_GROUP(TEXT("QxMultThread"), STATGROUP_QxMultiThread, STATCAT_Advanced); //定义命令名称，其中STATGROUP_QxTestGroup是stat命令输入的文字， TEXT内是显示的，STATCAT_Advanced是固定写法
// DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("qx stat count"), STAT_COUNT, STATGROUP_QxTestGroup);
DECLARE_CYCLE_STAT(TEXT("QxTestDevide"), STAT_QxTestDevide, STATGROUP_QxMultiThread); //统计时间, STAT_MyTIME 是宏名和SCOPE_CYCLE_COUNTER关联

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

// Called when the game starts or when spawned
void AQxMultithreadTest::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AQxMultithreadTest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Test();
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

