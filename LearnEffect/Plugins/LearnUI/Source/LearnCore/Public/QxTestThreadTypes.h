#pragma once

#include "CoreMinimal.h"

enum class EQxTestLog : uint8;
class FQxTestShared;
class AQxTestRunnable;
class AQxMultithreadTest;

class FQxTestRunnable : public FRunnable
{
public:
    FQxTestRunnable(
        FString InThreadName,
        TPromise<float>&& InPromise,
        AQxTestRunnable* InTester,
        FQxTestShared* InTestShared);

    virtual bool Init() override;
    virtual uint32 Run() override;

    // 被kill 走这条路径
    virtual void Stop() override;

    // 正常执行完  走这条路径
    virtual void Exit() override;
    virtual ~FQxTestRunnable() ;

private:
    // 下面是几种不同的interleave的更新共享值的测试
#pragma region NAME
    // 互斥锁实现
    void InterleaveUpdate1();

    // FVent 实现
    void InterleaveUpdate2();

    // atomic实现
    void InterleaveUpdate3();

    // 和atomic 实现，不过这个加入内存模型的考量
    void InterleaveUpdate4();

    // spinlock 实现
    // #TODO UE4 好像没有spin lock的实现，需要用其他方法
    void InterleaveUpdate5();

    // 通过共享的线程安全的队列实现 #TODO
    void InterleaveUpdate6();
#pragma endregion
public:
    const FString MyThreadName;
    AQxTestRunnable* Tester;
    // 用来保护Tester值的更新
    static FCriticalSection CriticalSection;

    static constexpr int32 MaxThreadCount = 2;

    // 第一种FEvent的用法
    FEventRef UpdateEvent;

    // 这个用来通知另一个对象，进行interleave的更新
    FQxTestRunnable* OtherTestRunnable;

    // 这个用来处理interleave相关
    int32 ThreadIndex;

private:
    FQxTestShared* TestShared;

    TPromise<float> TestPromise;
private:
    // 预期下个会更新值的线程 index 
    static  int32 ExpectNextThreadIndex;

    // 用来决定更新thread0 还是thread1
    static TAtomic<int32> bToUpdateThreadIndex;
};


// 用来复制一份AQxTestRunnable的数据
class FQxTestShared
{
public:
    // 只有这个需要更新，其他全部是const,即只有这个的访问需要保护
    TAtomic<int32> AtoTestCounter = 0;

    const int32 CounterMax;

    // const bool bPrintVerboseLog;
    const EQxTestLog TestLogLevel;
	
    const bool bNeverStop;

    const bool bWaitThreadComplete;
};


