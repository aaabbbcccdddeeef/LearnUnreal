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

namespace
{
    // 主要参考HLSLUtils.h中的实现，这里没有用TArray管理多page,
    // 不支持释放单个元素
    class FQxLinearAllocatorInternal
    {
    public:
        static constexpr SIZE_T MinPageSize =  64 * 1024;
        FQxLinearAllocatorInternal(SIZE_T InSize)
        {
            Reset(InSize);
        }

        void FreeAll()
        {
            if (Begin)
            {
                delete[] Begin;
            }
            Begin = Current = End = nullptr;
        }

        static FQxLinearAllocatorInternal* Allocate()
        {
            return new FQxLinearAllocatorInternal(MinPageSize);
        }
        
        
        void Reset(SIZE_T newSize)
        {
            if (newSize <= (SIZE_T)(End - Begin)) //这种情况不需要重分配内存
            {
                Begin = Current = End = nullptr;
                return;;
            }
            FreeAll();
            
            Begin = new  int8[MinPageSize];
            End = Begin + MinPageSize;
            Current = Begin;
        }
    private:

    private:
        int8* Current = nullptr;
        int8* Begin = nullptr;
        int8* End = nullptr;
    };
    
    
    /**
     * @brief 参考UE4的TInlineAllocator尝试实现一个兼容TArray的自定义的Allocator
     * 这里的LinearAllocator思想是：
     * Reallocate时分配固定大小的堆内存，free时不真正释放，
     */
    class TQxLinearAllocatorPolicy
    {
    public:
        using SizeType = int32;

        // Unreal allocator magic
        enum { NeedsElementType  = false };
        enum { ReuireRangeCheck = true  };

        template<typename ElementType>
        class ForElementType
        {
        public:
            ForElementType()
            {
                LinearAllocatorInternal = FQxLinearAllocatorInternal::Allocate();
            }

            ~ForElementType()
            {
                LinearAllocatorInternal->FreeAll();
                LinearAllocatorInternal =  nullptr;
            }
            

        private:
            ForElementType(const ForElementType&);
            ForElementType& operator=(const ForElementType&);

            // 用来计算增长容量时slack 容量增长的计算
            SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElement,
                SIZE_T NumBytesPerElement) const
            {
                // 采用引擎一般默认的分配策略
                return DefaultCalculateSlackGrow(NumElements, NumAllocatedElement, NumBytesPerElement, false);
            }
            SizeType CalculateSlackShrink(SizeType NumElements, SIZE_T NumAllocatedElement,
                SIZE_T NumBytesPerElement) const
            {
                return DefaultCalculateSlackShrink(NumElements, NumAllocatedElement, NumBytesPerElement, false);
            }

            SizeType CalculateSlackReserve(SizeType NumElements, SIZE_T NumBytesPerElement) const
            {
                return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, false);
            }

            // container 调用这个来获得container占用的内存，
            SIZE_T GetAllocatedSize(SizeType NumAllocatedElement, SIZE_T NumBytesPerElement) const
            {
                return NumAllocatedElement * NumBytesPerElement;
            }

            void ResizeAllocation(SizeType PreviousNumElements, SizeType NumElements, SIZE_T NumBytesPerElement)
            {
                
            }

            bool HasAllocation()
            {
                return  !!Data;
            }

            // // 已分配的内存的起始地址
            // void* m_StartPtr = nullptr;
            // // 当前已分配的最后一个对象结束的相对起始的offset
            // SIZE_T m_offset  = nullptr;
            FQxLinearAllocatorInternal* LinearAllocatorInternal = nullptr;

        private:
            ElementType* Data = nullptr;
        };

        typedef void ForAnyElementType;

    private:

    };


    // template<uint32 NumElements>
    // 只是将内存分配转发给global 的allocator，没什么用，只是为了熟悉allocator的写法
    // 但这里可以看出allocator至少需要track 已经分配的内存
    class FQxSimpleAllocator
    {
    public:
        
        using SizeType = int32;

        enum  { NeedsElementType = true  };

        enum  {  RequireRangeCheck = true }; // 这里需要是true 吗???#TODO

        template<typename  ElementType>
        class ForElementType
        {
        public:
            static constexpr SizeType InitCapcity = 10;
            ForElementType() {  }

            ElementType* GetAllocation() const
            {
                return Data;
            }

            void ResizeAllocation(SizeType PreviousNumElements,
                SizeType NumElement,
                SIZE_T NumBytesPerElement)
            {
                if (Data)
                {
                    GMalloc->Free(Data);
                }

                Data = static_cast<ElementType*>(GMalloc->Malloc(NumElement * NumBytesPerElement));

                checkf(Data, TEXT("Data allocate failled"));
            }

            // 用来计算增长容量时slack 容量增长的计算
            SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElement,
                SIZE_T NumBytesPerElement) const
            {
                // 采用引擎一般默认的分配策略
                return DefaultCalculateSlackGrow(NumElements, NumAllocatedElement, NumBytesPerElement, false);
            }
            SizeType CalculateSlackShrink(SizeType NumElements, SIZE_T NumAllocatedElement,
                SIZE_T NumBytesPerElement) const
            {
                return DefaultCalculateSlackShrink(NumElements, NumAllocatedElement, NumBytesPerElement, false);
            }

            SizeType CalculateSlackReserve(SizeType NumElements, SIZE_T NumBytesPerElement) const
            {
                return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, false);
            }
            
            SIZE_T GetAllocatedSize(SizeType NumAllocatedElements, SIZE_T NumBytesPerElement) const
            {
                return NumAllocatedElements * NumBytesPerElement;
            }

            // 这个应该是表示是否分配了堆内存
            bool HasAllocation() const
            {
                return !!Data;
            }

            // 这个表示构造完成之后容量，由于这个allocator只是一个全局allocator的转发，初始时没有内存分配
            SizeType GetInitialCapacity() const
            {
                return 0;
            }

        private:
            ForElementType(const ForElementType&);
            ForElementType& operator=(const ForElementType&);

            ElementType* Data = nullptr;
        };

        typedef void ForAnyElementType;

    };

    template<typename ElementType, typename  Allocator>
    void PrintTArray(const TArray<ElementType, Allocator>& InArray)
    {
        FString outStr = TEXT("{");
        for (auto& Element : InArray)
        {
            outStr += FString::Printf(TEXT("%d,"), Element);
        }
        outStr += "}";
        UE_LOG(LogTemp, Warning, TEXT("Array value = %s"), *outStr);
    }
}

void ATestUObjectLife::TestQxTestAllocaotr()
{
    // TArray<int32, TQxLinearAllocatorPolicy> test;
    TArray<int32 ,FQxSimpleAllocator> test;

    for (int32 i = 0; i < 1000; ++i)
    {
        test.Add(i);
    }
    // 下面的调用如果参数不加allocator编译不过，说明写入模板参数的不同allocator不是同类型
    PrintTArray(test);

    test.Reset();
    test.Reserve(100);
    test.AddDefaulted(20);
    for (auto& Element : test)
    {
        Element = FMath::RandRange(0, 800);
    }
    PrintTArray(test);
}
