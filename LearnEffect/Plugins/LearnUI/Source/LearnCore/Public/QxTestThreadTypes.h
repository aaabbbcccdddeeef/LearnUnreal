#pragma once

#include "CoreMinimal.h"

class AQxTestRunnable;
class AQxMultithreadTest;

class FQxTestRunnable : public FRunnable
{
public:
    FQxTestRunnable(FString InThreadName, AQxTestRunnable* InTester)
        : MyThreadName(InThreadName), Tester(InTester)
    {
    }
    
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;
    virtual ~FQxTestRunnable() ;

    const FString MyThreadName;
    AQxTestRunnable* const Tester;
    // 用来保护Tester值的更新
    static FCriticalSection CriticalSection;
private:
};
