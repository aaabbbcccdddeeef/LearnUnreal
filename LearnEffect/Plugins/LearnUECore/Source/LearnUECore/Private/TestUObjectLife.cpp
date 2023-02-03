// Fill out your copyright notice in the Description page of Project Settings.


#include "TestUObjectLife.h"

#include "LearnUECore.h"


namespace 
{

    
}

void UTestObjectLife::TestOuterAndPackage(UObject* InObject)
{
    
    while (UObject* targetObj = InObject->GetOuter())
    {
        UE_LOG(LogTemp, Warning, TEXT("Object name = %s"), *targetObj->GetName());
    }

}

// void UTestObjectLife::TestMemoryAllocate()
// {
//    
// }


// Sets default values
ATestUObjectLife::ATestUObjectLife()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATestUObjectLife::BeginPlay()
{
    Super::BeginPlay();
    
}

// Called every frame
void ATestUObjectLife::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATestUObjectLife::CreateTestObject()
{
    UQxTestClass* tmp = NewObject<UQxTestClass>(this, TEXT("QxTestObj"));
    TestObject = tmp;
}

void ATestUObjectLife::TestMemoryAllocate()
{
    using namespace QxTest;
    constexpr uint32 TestSize = 1;
    // 通过GMalloc进行操作
    {
        
        void* testPtr = GMalloc->Malloc(sizeof(FMyTestClass) * TestSize);
        FMyTestClass* myPtr = reinterpret_cast<FMyTestClass*>(testPtr);
        myPtr->Test1 = 100;

        // GMalloc->Free(testPtr);
        Test1 = myPtr;
    }

    
    // 通过FMemory 进行操作
    {
        void* testPtr = FMemory::Malloc(sizeof(FMyTestClass) * TestSize);
        FMyTestClass* myPtr = reinterpret_cast<FMyTestClass*>(testPtr);
        myPtr->Test1 = 200;

        Test2 = myPtr;
    }

    // 使用new/delete
    {

        Test3 = new FMyTestClass();
        // Test3 = myPtr;
    }
}

void ATestUObjectLife::TestFree()
{
    GMalloc->Free(Test1);
    Test1 =  nullptr;

    FMemory::Free(Test2);
    Test2 = nullptr;

    delete Test3;
}

void ATestUObjectLife::TestContainterAllocators()
{
    {
        TArray<int32, TInlineAllocator<2>> lTest1;

        TArray<int32, TInlineAllocator<4>> lTest2;

        TArray<int32, TInlineAllocator<6>> lTest3;

        TArray<int32, FDefaultAllocator> lTest4;

        int32 size1, size2, size3, size4;
        size1 = sizeof(lTest1);
        size2 = sizeof(lTest2);
        size3 = sizeof(lTest3);
        size4 = sizeof(lTest4);
        UE_LOG(LearnUECore, Warning, TEXT("inlineAllocator1 size = {%d}"), size1);
        UE_LOG(LearnUECore, Warning, TEXT("inlineAllocator2 size = {%d}"), size2);
        UE_LOG(LearnUECore, Warning, TEXT("inlineAllocator3 size = {%d}"), size3);
        UE_LOG(LearnUECore, Warning, TEXT("inlineAllocator4 size = {%d}"), size4);
        // TInlineAllocator<10> TestAllocator;
        // TInlineAllocator<10>::ForElementType<int32> lTest1;
        // lTest1.ResizeAllocation()
    }
}
