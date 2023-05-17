// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestUObjectLife.generated.h"

namespace QxTest
{
class FMyTestClass
{
public:
    FMyTestClass()
    {
        Test1 = 100;
    }

    ~FMyTestClass()
    {
        Test1 = 1;
    }
        
    int32 Test1 = 0;
};
    
}

UCLASS(Blueprintable)
class UQxTestClass : public UObject
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, Category="Test")
    int32 Test1 = 0;

    UPROPERTY(VisibleAnywhere,Transient)
    int32 Test2 = 0;
};

UCLASS(Blueprintable)
class UQxTestClass2 : public UObject
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, Category="Test")
    int32 Test1 = 0;

    UPROPERTY(VisibleAnywhere,Transient)
    int32 Test2 = 0;
};

UCLASS()
class UTestObjectLife : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    // 测试UObject的outer层级关系，以及最外层是不是package
    UFUNCTION(BlueprintCallable, Category="Test")
    static  void TestOuterAndPackage(UObject* InObject) ;


    // // 测试UE4的内存分配
    // UFUNCTION(BlueprintCallable, Category="Test")
    // static  void TestMemoryAllocate();
};

// 这个类主要用来测试UObject对象的生命周期
UCLASS(Blueprintable)
class LEARNUECORE_API ATestUObjectLife : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATestUObjectLife();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

   
    UFUNCTION(BlueprintCallable, Category="Test")
       void CreateTestObject();


    // 测试UE4的内存分配
    UFUNCTION(BlueprintCallable, Category="Test")
    void TestMemoryAllocate();

    // 测试内存回收
    UFUNCTION(BlueprintCallable, Category="Test")
    void TestFree();

    UFUNCTION(BlueprintCallable, Category="Test")
    void TestContainterAllocators();

    UFUNCTION(BlueprintCallable, Category="Test")
    void TestQxTestAllocaotr();
protected:

    UPROPERTY(VisibleAnywhere, Category="Test")
    UQxTestClass* TestObject = nullptr;

    QxTest::FMyTestClass* Test1 = nullptr;

    QxTest::FMyTestClass* Test2 = nullptr;

    QxTest::FMyTestClass* Test3 = nullptr;

};




