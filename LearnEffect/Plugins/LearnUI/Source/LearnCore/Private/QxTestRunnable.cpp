// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestRunnable.h"
#include "LearnCore.h"
#include "QxTestThreadTypes.h"

AQxTestRunnable::AQxTestRunnable()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AQxTestRunnable::BeginPlay()
{
    Super::BeginPlay();

    // 这里用智能指针应该是更合理的选择，但测试为了排除智能指针的线程安全的问题
    TestShared = new FQxTestShared{TestCounter, TestCounter, CounterMax, TestLogLevel, bNeverStop,bWaitThreadComplete}; 

    TPromise<float> TestPromise1;
    TPromise<float> TestPromise2;
    TestFuture1.Reset();
    TestFuture2.Reset();
    TestFuture1 = TestPromise1.GetFuture();
    TestFuture2 = TestPromise2.GetFuture();
    FQxTestRunnable* Runnable1 = new FQxTestRunnable(TEXT("TestThread0"), MoveTemp(TestPromise1) , this, TestShared, true);
    FQxTestRunnable* Runnable2 = new FQxTestRunnable(TEXT("TestThread1"), MoveTemp(TestPromise2), this, TestShared, false);
    Runnable1->OtherTestRunnable = Runnable2;
    Runnable2->OtherTestRunnable = Runnable1;

    
    Runnable1->UpdateEvent->Trigger();
    RunnableThread1 = FRunnableThread::Create(Runnable1, *Runnable1->MyThreadName);
    RunnableThread2 = FRunnableThread::Create(Runnable2, *Runnable2->MyThreadName);

    


    if (bWaitThreadComplete)
    {
        UE_LOG(LogTemp, Warning, TEXT("TestFuture1 = %f"), TestFuture1.Get()) ;
        UE_LOG(LogTemp, Warning, TEXT("TestFuture2 = %f"), TestFuture2.Get()) ;
        // RunnableThread1->WaitForCompletion();
        // if (Runnable2)
        // {
        //     RunnableThread2->WaitForCompletion();
        // }
    }
    
}

void AQxTestRunnable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // #TODO  怎样合理的回收线程资源
    if (RunnableThread1)
    {
        // 这里应该挂起还是 kill
        RunnableThread1->Kill(false);
        // RunnableThread1->Suspend();
        delete RunnableThread1;
        RunnableThread1 = nullptr;
    }
    if (RunnableThread2)
    {
        RunnableThread2->Kill(false);
        // RunnableThread2->Suspend();
        delete RunnableThread2;
        RunnableThread2 = nullptr;
    }

    // if (TestShared)
    // {
    //     delete TestShared;
    //     TestShared = nullptr;
    // }
    Super::EndPlay(EndPlayReason);
}

void AQxTestRunnable::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // if (TestFuture1.IsValid() && TestFuture1.IsReady())
    // {
    //     float FutureResult = TestFuture1.Get();
    //     UE_LOG(LearnCore, Warning, TEXT("Future Result1 = %f"), FutureResult);
    // }
    //
    // if (TestFuture2.IsValid() && TestFuture2.IsReady())
    // {
    //     float FutureResult = TestFuture2.Get();
    //     UE_LOG(LearnCore, Warning, TEXT("Future Result2 = %f"), FutureResult);
    // }
}

void AQxTestRunnable::KillTestThreads(bool WaitFinish)
{
    if (RunnableThread1)
    {
        RunnableThread1->Kill(WaitFinish);
    }
    if (RunnableThread2)
    {
        RunnableThread2->Kill(WaitFinish);
    }
    RunnableThread1 = nullptr;
    RunnableThread2 = nullptr;
}

void AQxTestRunnable::SusppendTestThreads()
{
    if (RunnableThread1)
    {
        RunnableThread1->Suspend();
    }
    if (RunnableThread2)
    {
        RunnableThread2->Suspend();
    }
}

void AQxTestRunnable::ResumeTestThreads()
{
    if (RunnableThread1)
    {
        RunnableThread1->Suspend(false);
    }
    if (RunnableThread2)
    {
        RunnableThread2->Suspend(false);
    }
}
