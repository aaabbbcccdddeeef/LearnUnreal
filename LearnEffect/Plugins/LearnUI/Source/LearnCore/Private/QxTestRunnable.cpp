// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestRunnable.h"
#include "LearnCore.h"
#include "QxTestThreadTypes.h"

void AQxTestRunnable::BeginPlay()
{
    Super::BeginPlay();

    FQxTestRunnable* Runnable1 = new FQxTestRunnable(TEXT("TestThread1"), this);
    FQxTestRunnable* Runnable2 = new FQxTestRunnable(TEXT("TestThread2"), this);

    RunnableThread1 = FRunnableThread::Create(Runnable1, *Runnable1->MyThreadName);
    RunnableThread2 = FRunnableThread::Create(Runnable2, *Runnable2->MyThreadName);

    if (bWaitThreadComplete)
    {
        RunnableThread1->WaitForCompletion();
        RunnableThread2->WaitForCompletion();
    }
}

void AQxTestRunnable::KillTestThreads()
{
    if (RunnableThread1)
    {
        RunnableThread1->Kill();
    }
    if (RunnableThread2)
    {
        RunnableThread2->Kill();
    }
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
