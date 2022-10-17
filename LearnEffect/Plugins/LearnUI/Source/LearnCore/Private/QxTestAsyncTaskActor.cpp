// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestAsyncTaskActor.h"

#include "LearnCore.h"
#include "QxTestAsyncTask.h"


// Sets default values
AQxTestAsyncTaskActor::AQxTestAsyncTaskActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxTestAsyncTaskActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AQxTestAsyncTaskActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 在tick这里进行task释放或许有些晚，但先这样做
    if (TestAsyncTask.IsValid() && TestAsyncTask->IsDone())
    {
        TestAsyncTask.Reset();
        double EndTime = FPlatformTime::Seconds();
        UE_LOG(LearnCore, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (EndTime-StartTime)*1000);
    }
}

void AQxTestAsyncTaskActor::TestAsyncTaskClass1()
{
    StartTime = FPlatformTime::Seconds();

    (new FAutoDeleteAsyncTask<FQxTestAsyncTask>(TargetCounter))->StartBackgroundTask();
    double EndTime = FPlatformTime::Seconds();
    UE_LOG(LearnCore, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (EndTime-StartTime)*1000);
}

void AQxTestAsyncTaskActor::TestAsyncTaskClass2()
{
    StartTime = FPlatformTime::Seconds();
    TestAsyncTask.Reset();
    TestAsyncTask =  MakeUnique<FAsyncTask<FQxTestAsyncTask>>(TargetCounter);
    TestAsyncTask->StartBackgroundTask();
}

void AQxTestAsyncTaskActor::TestAsyncTaskFunc_AnyThread()
{
    StartTime = FPlatformTime::Seconds();
    AsyncTask(ENamedThreads::AnyThread, [=]()
    {
        double Result = 0;
        for (int32 i = 0; i < TargetCounter; i++)
        {
            Result += FMath::Sqrt(i) / TargetCounter;
        }

        UE_LOG(LogTemp, Warning, TEXT("@%u wait TargetCounterR(%f)"), __LINE__, Result);

        double End = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Warning, TEXT("@%u wait millisecond(%f) end."), __LINE__, (End-StartTime)*1000);
    });
    double End = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (End-StartTime)*1000);
}

void AQxTestAsyncTaskActor::TestAsyncTaskFunc_GameThread()
{
    StartTime = FPlatformTime::Seconds();
    AsyncTask(ENamedThreads::GameThread, [=]()
    {
        double Result = 0;
        for (int32 i = 0; i < TargetCounter; i++)
        {
            Result += FMath::Sqrt(i) / TargetCounter;
        }

        UE_LOG(LogTemp, Warning, TEXT("@%u wait TargetCounterR(%f)"), __LINE__, Result);

        double End = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Warning, TEXT("@%u wait millisecond(%f) end."), __LINE__, (End-StartTime)*1000);
    });
    double End = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (End-StartTime)*1000);
}

void AQxTestAsyncTaskActor::TestAsyncFunc_NoReturn()
{
    UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
    StartTime = FPlatformTime::Seconds();

    auto CompleteCallback = []()
    {
        // UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
    };
    Async(EAsyncExecution::TaskGraph, [=]()
    {
        double Result = 0;
        for (int32 i = 0; i < TargetCounter; i++)
        {
            Result += FMath::Sqrt(i) / TargetCounter;
        }
        UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId())
        UE_LOG(LogTemp, Warning, TEXT("@%u wait TargetCounterR(%f)"), __LINE__, Result);
        double End = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Warning, TEXT("@%u wait millisecond(%f) end."), __LINE__, (End-StartTime)*1000);
        UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
        AsyncTask(ENamedThreads::GameThread, []()
       {
           UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
       });
    }, MoveTemp(CompleteCallback));
    
    double EndTime = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (EndTime-StartTime)*1000);
}

void AQxTestAsyncTaskActor::TestAsyncFunc_WithFuture()
{
    UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
    StartTime = FPlatformTime::Seconds();
    auto CompleteCallback = []()
    {
        AsyncTask(ENamedThreads::GameThread, []()
        {
            UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
        });
    };
    // TUniqueFunction<void()> TmpFunc =  TFunction<void()>(this, &AQxTestAsyncTaskActor::OnAsyncFuncFinish);
    TFuture<double> FutureResult = Async(EAsyncExecution::TaskGraph, [=]()
    {
        double Result = 0;
        for (int32 i = 0; i < TargetCounter; i++)
        {
            Result += FMath::Sqrt(i) / TargetCounter;
        }
		
        double End = FPlatformTime::Seconds();
        UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId())
        UE_LOG(LogTemp, Warning, TEXT("@%u wait millisecond(%f) end."), __LINE__, (End-StartTime)*1000);
        return Result;
    }, MoveTemp(CompleteCallback));
    //使用Get获取返回值，会阻塞主线程
    UE_LOG(LogTemp, Warning, TEXT("@%u wait TargetCounterR(%f)"), __LINE__, FutureResult.Get());
    double End = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (End-StartTime)*1000);
}

void AQxTestAsyncTaskActor::TestAsyncFunc_ParallelFor()
{
    StartTime = FPlatformTime::Seconds();

    const int32 Stride = TargetCounter / 10;
    auto FutureResult = Async(EAsyncExecution::TaskGraph, [=]()
    {
       TArray<double> ResultArray;
        ResultArray.Init(0, 10);
        ParallelFor(ResultArray.Num(), [&ResultArray, Stride, this](int32 ArrayIndex)
        {
            for (int32 i = ArrayIndex * Stride; i < (ArrayIndex + 1) * Stride; ++i)
            {
                ResultArray[ArrayIndex] += FMath::Sqrt(i)/TargetCounter;
            }
        });
        
       double Sum = 0;
       for (int32 i = 0; i < ResultArray.Num(); ++i)
       {
           Sum += ResultArray[i];
       }
        UE_LOG(LogTemp, Warning, TEXT("@%u wait TargetCounterR(%f)"), __LINE__, Sum);
         double End = FPlatformTime::Seconds();
         UE_LOG(LogTemp, Warning, TEXT("@%u wait millisecond(%f) end."), __LINE__, (End-StartTime)*1000);
         return Sum; 
    });

    float EndTime = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__"@%u wait millisecond(%f) main thread end."), __LINE__, (EndTime-StartTime)*1000);
}

void AQxTestAsyncTaskActor::OnAsyncFuncFinish()
{
    UE_LOG(LearnCore, Warning, TEXT("CurrentThreadId = %u"),  FPlatformTLS::GetCurrentThreadId());
}

