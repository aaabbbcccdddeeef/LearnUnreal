#pragma once


#include <assert.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <windows.h>

void TestAtomicFlag()
{
    using namespace std;
    
    std::atomic_flag flag1 = ATOMIC_FLAG_INIT;

    cout << "1. previous flag value " << flag1.test_and_set() << endl;
    cout << "2. previous flag value " << flag1.test_and_set() << endl;
    cout << "3. previous flag value " << flag1.test_and_set() << endl;

    flag1.clear();
    cout << "3. previous flag value " << flag1.test_and_set() << endl;
    
}


void TestAtomicBool()
{
    using namespace  std;
    atomic<bool> flag1;
    cout << "flag1 = " << flag1 << std::endl;
    flag1 = 0;

    // atomic<*> 不能复制构造或者赋值
    // atomic<bool> flag2(flag1);
    // atomic<bool> flag3 = flag1;
    
    // construct by using non atomic boolean value
    bool nonAtomicBool = true;
    atomic<bool> flag4(nonAtomicBool);
    cout << " flag4 = " << flag4 << endl;
    atomic<bool> flag5 = nonAtomicBool;
    cout << " flag5 = " << flag5 << endl;

}

void TestAtomicOperation()
{
    using namespace std;
    std::atomic<int> x(20);

    cout << "atomic int is lock free ? " << (x.is_lock_free() ? "yes" : "no");

    int expectedValue = 50;
    cout << "previous expectedValue " << expectedValue << std::endl;

    bool returnValue = x.compare_exchange_weak(expectedValue, 6);

    cout << "operation success " << (returnValue ? "Yes" : "NO") << std::endl;
    cout << "current expected value " << expectedValue << std::endl;
    cout << "currrent x " << x.load() << std::endl; 
}

namespace Test1
{
    using namespace std;

    atomic<bool> x,y;
    atomic<int> z;


    void WriteXThenY()
    {
        x.store(true, std::memory_order_relaxed);
        // y.store(true, std::memory_order_relaxed);
        y.store(true, std::memory_order_release);
    }

    
    
    void ReadXThenY()
    {
        while (!y.load(std::memory_order_acquire))
            ;

        if (x.load(std::memory_order_relaxed))
        {
            z++;
        }
    }
    
    void TesMemoryOrderRelax()
    {
        x = false;
        y = false;
        z = 0;

        thread writerThread(WriteXThenY);
        thread readThread(ReadXThenY);

        writerThread.join();
        readThread.join();

        assert(z != 0);
    }

    
}

