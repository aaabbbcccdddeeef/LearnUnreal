// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TestReflection.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Transient)
class UTestReflection : public UObject
{
public:
    GENERATED_BODY()
    UTestReflection(const FObjectInitializer& ObjectInitializer);

    
public:
    UPROPERTY(BlueprintReadWrite)
    float Score;
public:
    UFUNCTION(BlueprintCallable, Category = "Hello")
    void CallableFunc();    //C++实现，蓝图调用

    UFUNCTION(BlueprintNativeEvent, Category = "Hello")
    void NativeFunc();  //C++实现默认版本，蓝图可重载实现
    void NativeFunc_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Hello")
    void MyTestFunc();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Hello")
    void ImplementableFunc();   //C++不实现，蓝图实现
};
