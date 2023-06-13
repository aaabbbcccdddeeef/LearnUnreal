// Fill out your copyright notice in the Description page of Project Settings.


#include "TestReflection.h"

UTestReflection::UTestReflection(const FObjectInitializer& ObjectInitializer)
{
    UE_LOG(LogTemp, Warning, TEXT("TEST"));
}

void UTestReflection::CallableFunc()
{
    FCriticalSection test;
    MyTestFunc();
}

void UTestReflection::NativeFunc_Implementation()
{
}

void UTestReflection::MyTestFunc_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("TEST"));
}
